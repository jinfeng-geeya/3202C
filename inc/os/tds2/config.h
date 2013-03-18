/*********************************************************************
 *
 *  Copyright (C) 2003 ALi.  All Rights Reserved.
 *
 *  File:   Config.h
 *
 *  Contents:    defines the system configuration environment.
 *
 *  History:
 *      Date          		By        Reason
 *   =====		======    =======  =============
 *   10/06/2003 Neo Li	    Clean orginal configureation for m3325 & m6303
 *   10/08/2003	Jeff Wu	    Reform
 *
 *********************************************************************/

#ifndef _WDVD_PLATFORM_CONFIGURATION_H_
#define  _WDVD_PLATFORM_CONFIGURATION_H_

///////////////////////////////////////////////////////////////////////
// The build setting such as "ALi_M3325C_Module_1" is exported by build script
// "mk.sh".
///////////////////////////////////////////////////////////////////////
#if defined ALi_M3325C_Module_1

#define	BYTE_ORDER		BIG_ENDIAN
#define	PLATFORM		M3325C_PLATFORM

// Default OS setting
#define DEFAULT_TASK_STACK_SIZE	0x3000
#define DEFAULT_TASK_QUANTUM		10

#if 1
	#define PREEMPT						// allow preempt
#else
	#define NO_PREEMPT
#endif


//支持动态优先级
#if 1
	#define	DYNAMIC_PRIORITY			//动态优先级
#else
	#define	NO_DYNAMIC_PRIORITY
#endif

//#include "chipreg.h"

// END ALi_M3325C_Module_1
///////////////////////////////////////////////////////////////////////

#elif defined ALi_M3355_Module_1

#define	BYTE_ORDER		BIG_ENDIAN
#define	PLATFORM		M3355_PLATFORM

// Default OS setting
#define DEFAULT_TASK_STACK_SIZE	0x3000
#define DEFAULT_TASK_QUANTUM		10

#if 1
	#define PREEMPT						// allow preempt
#else
	#define NO_PREEMPT
#endif


//支持动态优先级
#if 1
	#define	DYNAMIC_PRIORITY			//动态优先级
#else
	#define	NO_DYNAMIC_PRIORITY
#endif

// END ALi_M3355_Module_1
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// BEGIN ALi_M6303C_Module_1
#elif defined ALi_M6303_Module_1

#define	BYTE_ORDER		LITTLE_ENDIAN
#define PLATFORM                  M6303_PLATFORM

// Default OS setting
#define DEFAULT_TASK_STACK_SIZE	0x3000
#define DEFAULT_TASK_QUANTUM		10

#if 1
	#define PREEMPT						// allow preempt
#else
	#define NO_PREEMPT
#endif


//支持动态优先级
#if 1
	#define	DYNAMIC_PRIORITY			//动态优先级
#else
	#define	NO_DYNAMIC_PRIORITY
#endif


//#include "chipreg.h"

// END ALi_M6303C_Module_1
///////////////////////////////////////////////////////////////////////


#elif defined ALi_M6307_Module_1

// All the configuration of M6307 Platform
//#include "Config6307.h"
#if 1
	#define PREEMPT						// allow preempt
#else
	#define NO_PREEMPT
#endif


//支持动态优先级
#if 1
	#define	DYNAMIC_PRIORITY			//动态优先级
#else
	#define	NO_DYNAMIC_PRIORITY
#endif

// All the configuration of M6307 Platform


#elif defined ALi_x86_Module
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define	BYTE_ORDER		LITTLE_ENDIAN
#define	PLATFORM		X86_PLATFORM

// Default OS setting
#define DEFAULT_TASK_STACK_SIZE	0x3000
#define DEFAULT_TASK_QUANTUM		10
#if 1
	#define PREEMPT						// allow preempt
#else
	#define NO_PREEMPT
#endif

//支持动态优先级
#if 0
	#define	DYNAMIC_PRIORITY			//动态优先级
#else
	#define	NO_DYNAMIC_PRIORITY
#endif

// END ALi_x86_Module
//////////////////////////////////////////////////////
#elif defined ALi_M3357_Module_1

#define	BYTE_ORDER		LITTLE_ENDIAN
#define PLATFORM             M3357_PLATFORM

// Default OS setting
#define DEFAULT_TASK_STACK_SIZE	0x3000
#define DEFAULT_TASK_QUANTUM		10

#if 1
	#define PREEMPT						// allow preempt
#else
	#define NO_PREEMPT
#endif


//支持动态优先级
#if 1
	#define	DYNAMIC_PRIORITY			//动态优先级
#else
	#define	NO_DYNAMIC_PRIORITY
#endif

// END ALi_M6303C_Module_1
///////////////////////////////////////////////////////////////////////

#endif	// End of configuration list
#endif	// End of File
