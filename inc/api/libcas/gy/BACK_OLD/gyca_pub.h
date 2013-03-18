/*******************************************************************************
** Notice: Copyright (c) 2008 Geeya corporation - All Rights Reserved 
********************************************************************************
** Description:
**                 public head file between CAS Library and STB application.
**
** File Rev     $Revision: \main\20060816_1.0_integration_branch\1 $
**
** Date         $Date:     Wednesday Augest 16 11:54:04 2006 $
** 
** File Name: gyca_pub.h
**
********************************************************************************/
#ifndef _GYCAS_PUB_H_
#define _GYCAS_PUB_H_

/* OUT, this is a macro  to identify a output parameter in function defination 
** IN   , this is a macro  to identify a input parameter in function defination
** INOUT, this is a macro to identify a input and output parameter in function defination 
*/
#define OUT			
#define IN							
#define INOUT		

#define 	GYTRUE  		(1)
#define 	GYFALSE 		(0)
#define	GYSUCCESS	(0)
#define 	GYFAILURE		(-1)
#ifndef		NULL
#define		NULL			(0)
#endif

/* 为了防止编译器编译时遇到没有用到的参数报警而定义的宏 */
#define UNUSED_PARAMETER(x) ((void)(x))      

/* 将网络字节转成无符号短型整数和无符号长整数*/
#define ntos(n)    	((GYU16)(((GYU16)n[0] << 8) | (GYU16)n[1]))
#define ntol(n)     ((GYU32)(((GYU32)n[0] << 24) | ((GYU32)n[1] << 16) | ((GYU32)n[2] << 8) | (GYU32)n[3]))
/* 将无符号短整数和无符号长整数转换成网络字节*/
#define ston(n,s) 	n[0] = (GYU8)(((GYU16)s >> 8) & 0xFF); n[1] = (GYU8)((GYU16)s & 0xFF)
#define lton(n,l)   n[0] = (GYU8)(((GYU32)l >> 24) & 0xFF); n[1] = (GYU8)(((GYU32)l >> 16) & 0xFF); n[2] = (GYU8)(((GYU32)l >> 8) & 0xFF); n[3] = (GYU8)((GYU32)l & 0xFF)

/*海尔51内核的机顶盒方案，如Hi2016E*/
//#define HAIR_STB_51_KERNEL

#ifdef HAIR_STB_51_KERNEL
	#define CA_FUNCPOSTFIX   	large reentrant/*对海尔的Hi2016E等方案，使用KEIL C51编译环境，每个函数需要可重入定义*/
	#define CA_FAR				 far /*大变量加far关键字，但是赋有初值时不能加far. 适应51的寻址方式*/
	typedef  long            		GYS32;
	typedef  unsigned long 	GYU32; 
#else 
	#define CA_FUNCPOSTFIX   
	#define CA_FAR			
	typedef  int            		GYS32;
	typedef  unsigned int         	GYU32;
#endif

typedef  unsigned int			GYSPECIAL; /*位域定义，默认为整型；指针使用*/
typedef  signed char       		GYS8;
typedef  signed short           	GYS16;
typedef unsigned char         	GYU8;
typedef unsigned short        	GYU16;
typedef unsigned char			GYBOOL;       



#endif    //_GYCAS_PUB_H_


