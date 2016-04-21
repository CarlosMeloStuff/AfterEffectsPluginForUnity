#ifndef utj_Debug_h
#define utj_Debug_h

#ifdef utjWindows
#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>

#define utjMaxSimLen MAX_SYM_NAME

namespace utj {

bool Demangle(const char *mangled, char *demangled, size_t buflen, bool name_only = false);

// Body: [](HMODULE mod) -> void
template<class Body>
inline void EachLoadedModules(const Body &body)
{
    HMODULE *modules;
    DWORD num_modules;
    DWORD requied_size;
    auto proc = ::GetCurrentProcess();

    ::EnumProcessModules(proc, nullptr, 0, &requied_size);
    num_modules = requied_size / sizeof(HMODULE);
    modules = new HMODULE[num_modules];
    ::EnumProcessModules(proc, modules, requied_size, &requied_size);
    for (DWORD i = 0; i < num_modules; ++i) {
        body(modules[i]);
    }
    delete[] modules;
}

} // namespace utj
#endif // utjWindows
#endif // utj_Debug_h
