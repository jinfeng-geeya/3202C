#include "version.h"
#ifdef _ALI_SYSDBG_ENABLE_
char libcore_banner[] = 
	"DBG Libcore version " LIBCORE_VER "@" LIBCORE_LABEL "("COMPILER")" "(" COMPILE_BY "@" UTS_VERSION")\n";
#else
char libcore_banner[] = 
	"Libcore version " LIBCORE_VER "@" LIBCORE_LABEL "("COMPILER")" "(" COMPILE_BY "@" UTS_VERSION")\n";
#endif
char* get_core_ver(void)
{
    return libcore_banner; 
}
