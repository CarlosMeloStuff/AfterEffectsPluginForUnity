#include "pch.h"
#include "Debug.h"

#ifdef utjWindows
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

namespace utj {

bool Demangle(const char *mangled, char *demangled, size_t buflen, bool name_only)
{
    if (name_only) {
        return ::UnDecorateSymbolName(mangled, demangled, (DWORD)buflen, UNDNAME_NAME_ONLY) != 0;
    }
    else {
        return ::UnDecorateSymbolName(mangled, demangled, (DWORD)buflen,
            UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ALLOCATION_MODEL | UNDNAME_NO_ALLOCATION_LANGUAGE |
            UNDNAME_NO_MS_THISTYPE | UNDNAME_NO_CV_THISTYPE | UNDNAME_NO_THISTYPE | UNDNAME_NO_ACCESS_SPECIFIERS |
            UNDNAME_NO_RETURN_UDT_MODEL) != 0;
    }
}

} // namespace utj
#endif // utjWindows
