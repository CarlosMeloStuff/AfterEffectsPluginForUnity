#ifndef PE_h
#define PE_h
namespace utj {

#ifdef utjWindows

// F: [](size_t size) -> void* : alloc func (e.g.: malloc)
template<class F>
inline bool MapFile(const char *path, void *&o_data, size_t &o_size, const F &alloc)
{
    o_data = nullptr;
    o_size = 0;
    if (FILE *f = fopen(path, "rb")) {
        fseek(f, 0, SEEK_END);
        o_size = ftell(f);
        if (o_size > 0) {
            o_data = alloc(o_size);
            fseek(f, 0, SEEK_SET);
            fread(o_data, 1, o_size, f);
        }
        fclose(f);
        return true;
    }
    return false;
}


inline bool TryLoadModule(const char *dllname)
{
    HMODULE mod = ::GetModuleHandleA(dllname);
    if (mod) { return true; }

    mod = ::LoadLibraryA(dllname);
    // should FreeLibrary(mod) here?
    return mod != nullptr;
}

inline void GetDLLPath(HMODULE mod, char *dst_path, size_t dst_size)
{
    ::GetModuleFileNameA(mod, dst_path, (DWORD)dst_size);
}


// for internal use
inline void RVAFillGap(size_t& ImageBase, DWORD RVA)
{
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(ImageBase + pDosHeader->e_lfanew);

    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
    for (size_t i = 0; i < pNtHeaders->FileHeader.NumberOfSections; ++i) {
        PIMAGE_SECTION_HEADER s = pSectionHeader + i;
        if (RVA >= s->VirtualAddress && RVA < s->VirtualAddress + s->SizeOfRawData) {
            pSectionHeader = s;
            break;
        }
    }

    DWORD gap = pSectionHeader->VirtualAddress - pSectionHeader->PointerToRawData;
    ImageBase -= gap;
}

// Body: [](const char *funcname, void *funcptr) -> void
// fill_gap: should be true if module is directly memory mapped file (= not loaded by LoadModule())
template<class Body>
inline void EnumerateDLLExports(HMODULE module, const Body &body, bool fill_gap = false)
{
    if (module == nullptr) { return; }

    size_t ImageBase = (size_t)module;
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) { return; }

    PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(ImageBase + pDosHeader->e_lfanew);
    DWORD RVAExports = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (RVAExports == 0) { return; }

    if (fill_gap) {
        RVAFillGap(ImageBase, RVAExports);
    }

    IMAGE_EXPORT_DIRECTORY *pExportDirectory = (IMAGE_EXPORT_DIRECTORY *)(ImageBase + RVAExports);
    DWORD *RVANames = (DWORD*)(ImageBase + pExportDirectory->AddressOfNames);
    WORD *RVANameOrdinals = (WORD*)(ImageBase + pExportDirectory->AddressOfNameOrdinals);
    DWORD *RVAFunctions = (DWORD*)(ImageBase + pExportDirectory->AddressOfFunctions);
    for (DWORD i = 0; i < pExportDirectory->NumberOfFunctions; ++i) {
        char *pName = (char*)(ImageBase + RVANames[i]);
        void *pFunc = (void*)(ImageBase + RVAFunctions[RVANameOrdinals[i]]);
        body(pName, pFunc);
    }
}

// Body: [](const char *dllname) -> void
// fill_gap: should be true if module is directly memory mapped file (= not loaded by LoadModule())
template<class Body>
inline void EnumerateDependentDLLs(HMODULE module, const Body &body, bool fill_gap = false)
{
    if (module == nullptr) { return; }

    size_t ImageBase = (size_t)module;
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) { return; }

    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(ImageBase + pDosHeader->e_lfanew);
    DWORD RVAImports = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (RVAImports == 0) { return; }

    if (fill_gap) {
        RVAFillGap(ImageBase, RVAImports);
    }

    IMAGE_IMPORT_DESCRIPTOR *pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*)(ImageBase + RVAImports);
    while (pImportDesc->Name != 0) {
        const char *pDLLName = (const char*)(ImageBase + pImportDesc->Name);
        body(pDLLName);
        ++pImportDesc;
    }
    return;
}

// Body: [](const char *dllname, const char *funcname, void *&funcptr) -> void
// fill_gap: should be true if module is directly memory mapped file (= not loaded by LoadModule())
template<class Body>
inline void EnumerateDLLImports(HMODULE module, const Body &body, bool fill_gap = false)
{
    if (module == nullptr) { return; }

    size_t ImageBase = (size_t)module;
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) { return; }

    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(ImageBase + pDosHeader->e_lfanew);
    DWORD RVAImports = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (RVAImports == 0) { return; }

    if (fill_gap) {
        RVAFillGap(ImageBase, RVAImports);
    }

    IMAGE_IMPORT_DESCRIPTOR *pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*)(ImageBase + RVAImports);
    while (pImportDesc->Name != 0) {
        const char *pDLLName = (const char*)(ImageBase + pImportDesc->Name);
        IMAGE_THUNK_DATA* pThunkOrig = (IMAGE_THUNK_DATA*)(ImageBase + pImportDesc->OriginalFirstThunk);
        IMAGE_THUNK_DATA* pThunk = (IMAGE_THUNK_DATA*)(ImageBase + pImportDesc->FirstThunk);
        while (pThunkOrig->u1.AddressOfData != 0) {
            if ((pThunkOrig->u1.Ordinal & 0x80000000) > 0) {
                DWORD Ordinal = pThunkOrig->u1.Ordinal & 0xffff;
                // nameless function
            }
            else {
                IMAGE_IMPORT_BY_NAME* pIBN = (IMAGE_IMPORT_BY_NAME*)(ImageBase + pThunkOrig->u1.AddressOfData);
                body(pDLLName, (char*)pIBN->Name, *(void**)pThunk);
            }
            ++pThunkOrig;
            ++pThunk;
        }
        ++pImportDesc;
    }
    return;
}

#endif // utjWindows

} // namespace utj
#endif // PE_h
