#ifndef __PLATFORM_H__
#define __PLATFORM_H__

//************************ Release Mode definition ******************************//
//#define _DEB                 1

#ifdef _ALI_SYSDBG_ENABLE_
#define _DEB				1
#endif
// Include configuration file for current build.
#include "config.h"

#endif

