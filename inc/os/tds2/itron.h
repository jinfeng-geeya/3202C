#ifndef _ITRON_H_
#define _ITRON_H_

#ifdef __cplusplus
extern "C"{
#endif
#include <sys_config.h>
//Compiler switch to open/close debug message in TDS module
#include "alitypes.h"
#include "platform.h"

#if (SYS_CHIP_MODULE == ALI_S3601)
#define IRQ_NUM						8
#define EIRQ_NUM					32
#define TIMER_IRQ					7
#define IRQ_INTC_NUMBER				2
#define	IRQ_NUMBER_IR				15

#elif (SYS_CHIP_MODULE == ALI_S3602)
	#if((SYS_CPU_MODULE == CPU_M6303) && (SYS_PROJECT_FE==PROJECT_FE_DVBT))//for 3105
		#define EIRQ_NUM				32
	#else
		#define EIRQ_NUM				64
	#endif
#define IRQ_NUM						8
#define TIMER_IRQ					7
#define IRQ_INTC_NUMBER				3
#define	IRQ_NUMBER_IR				15
#else
#define IRQ_NUM						8
#define EIRQ_NUM					32
#define TIMER_IRQ					7
#define IRQ_INTC_NUMBER				3
#define	IRQ_NUMBER_IR				15

#endif

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// VOIDPRINT is a function that generate no debug message. In fact,
// it does nothing. To improve system performance, we should replace
// this function with "#define VOIDPRINT(...)" in the future. Now
// this definition could improve compiler compatibility.
#define VOIDPRINT	_VoidPrint
void _VoidPrint(const char *fmt, ...);
struct os_config{

	UINT16 task_num;
	UINT16 sema_num;
	UINT16 flg_num;
	UINT16 mbf_num;
	UINT16 mutex_num;
	UINT16 parity_value;
};
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// Output ERROR message
//#define ERRMSG	DEBUGPRINTF

#ifdef _DEB

	#define _TDS_TASK_PERFORMANCE	//用于统计每个TASK所占用的
									//CPU时间

	#define _TDS_INFO_MONITOR	//added by jeff wu,for tds monitor

	//#if (PLATFORM!=X86_PLATFORM)
		#define _TDS_PERFORMANCE			//added by steven
//		#define _TDS_HSR_PERFORMANCE		//added by steven,for HSR performance measure
//		#define _TDS_STACK_PERFORMANCE		//added by steven,for stack performance measure
//		#define _TDS_COUNT_PERFORMANCE		//added by steven,for dispatch and interrupt performance measure
	//#endif

	//#define _DEBUG_PRINTF_

	//#define DEBUGPRINTF	_DbgPrint
	//void _DbgPrint(const char *fmt, ...);

	extern UINT g_CodeBegin;
	extern UINT g_CodeEnd;

#else // #ifdef _DEB

//	#define ENTER_FUNCTION
//	#define LEAVE_FUNCTION
//	#define ASSERT(exp)
//	#define ASSERT_ADDR(address)
//	#define DEBUGPRINTF	VOIDPRINT

#endif // #ifdef _DEB

	typedef struct debug_control_block
	{
		UINT16	direct;
		UINT32	head;
		UINT32	rear;
		UINT32	length;
		UINT32	index;
	} DBGCB;

	#define	DEBUG_TO_MONITOR	0
	#define	DEBUG_TO_MEMORY	1
	#define	DEBUG_MEMORY_LENGTH	0x8000
	#define	DEBUG_MEMORY_OFFSET	0X20
	//added by jeff wu ,for control debug

#ifdef _TDS_INFO_MONITOR
	#define MONITOR_SYS 0x00000001
	#define MONITOR_TCB 0x00000002
	#define MONITOR_READY 0x00000004
	#define MONITOR_WAIT 0x00000008
	#define MONITOR_MEMORY 0x00000010
	#define MONITOR_FLAG 0x00000020
	#define MONITOR_SIGNAL 0x00000040
	#define MONITOR_SEMAPHORE 0x00000080
	#define MONITOR_MESSAGE 0x00000100
	#define MONITOR_CPU	0x00000200
	#define MONITOR_TASK_CPU_Measure	0x00000400
    #define OS_ASSERT ASSERT
#else
    #define OS_ASSERT(...)	  do{}while(0)
#endif


////////////////// Memory block monitor interface //////////////////////////
// added by Speer, 2004.3.2

/*
 *  Example:
 *    ...
 *    TDS_MemMon_Begin();                            \
 *    ...                                            \
 *    TDS_MemMon_CurStat();     // seldom used, :)   \
 *    ...                                            \
 *    if (0 != TDS_MemMonLeakOccurs()) {             check body/duration
 *        TDS_MemMon_DumpLeak();                     /
 *    }                                              /
 *    TDS_MemMon_Stop();                             /
 *    ...
 *    TDS_MemMonBegin();
 *    ...
 */

void TDS_MemMon_Begin(void);       // start the monitor;
void TDS_MemMon_Stop(void);        // stop and release the monitor, but doesn't
                                   // de-allocate the leak blocks
/* Function Description: [TDS_MemMon_LeakOccurs]
     Check whether memory leak occurs in current check point;
   Return Value:
     If return 0, none leak occurs;
     Otherwise, the number of no-freed blocks, always positive;
 */
int  TDS_MemMon_LeakOccurs(void);  // if return 0, no leak; otherwise,
                                   // number of leak blocks; always positive
void TDS_MemMon_DumpLeak(void);    // Dump the no-freed block;
void TDS_MemMon_DumpCurStat(void); // Dump current no-freed block;

////////////////// Memory block monitor interface //////////////////////////


//////////////////////////////////////////////////////////////////
// Special type definition for ITRON RTOS
typedef short              BOOL_ID;
typedef short              H;
typedef char               B;
typedef char               VB;
typedef short              HNO;
typedef WORD               ID;
typedef ID                 ID_TASK;
typedef ID                 ID_THREAD;
typedef ID                 ID_MODULE;    // Software module ID
typedef ID                 ID_INT;       /* interrupt ID */
typedef ID                 ID_HSR;       /* high service routine ID */
typedef ID                 ID_SIGNAL;
typedef ID                 ID_STRUCTYPE; /* struct type */
typedef short              PRI;

#define	INVALID_ID         0xFFFF

// Common functions
typedef void               (*T_VOID_FUNC_PTR)(void);
typedef void               (*T_SIG_PROC_FUNC_PTR)(ID_SIGNAL, DWORD, DWORD);
typedef void               (*T_HSR_PROC_FUNC_PTR)(DWORD);

typedef unsigned long      ATR;
typedef unsigned long      TMO;
typedef unsigned long      UW;

typedef long               ER;
typedef long               VW;  /* variable data type (32 bit) */

//typedef void               (*FP)(void);
typedef void (*FP)(DWORD, DWORD);
typedef void (*TP)(UINT,UINT);
typedef void (*VOIDFP)(void);

#define NADR               ((VP)-1)

#define TA_ASM             0    /* Assembler language program */
#define TA_HLNG            1    /* High-level language program */

#define RSN_TIMEOUT        0
#define RSN_BLOCK          1



/* System call Error Code */
#define E_OK               0
#define E_FAILURE		(-1)
#define E_TIMEOUT	(-2)

////////////////// Create Fail  //////////////////
enum create_err
{
	_CREATE_TSK_FAIL = 0x10000001,	// Create Task Fail
	_CREATE_FLG_FAIL,    			// Create Flag Fail
	_CREATE_SEM_FAIL,    			// Create Semaphor Fail
	_CREATE_MSG_FAIL,    			// Create Message Fail
	_CREATE_MBX_FAIL,    			// Create Mail box Fail
	_CREATE_ALM_FAIL    			// Create Timer Fail
};
////////////////// Release Fail  //////////////////
enum delete_err
{
	_DELETE_TSK_FAIL = 0x11000001, 	// Delete Task Fail
	_DELETE_FLG_FAIL,  				// Delete Flag Fail
	_DELETE_SEM_FAIL,  				// Delete Semaphor Fail
	_DELETE_MSG_FAIL,  				// Delete Message Fail
	_DELETE_MBX_FAIL,  				// Delete Mail box Fail
	_DELETE_ALM_FAIL,  				// Delete Timer Fail
};

////////////////// Set Fail  //////////////////
enum seting_err
{
	_SET_SEM_FAIL = 0x12000001,     // Set Semaphor Fail
	_SEND_MSG_FAIL,   				// Send Message Fail
	_STA_TSK_FAIL,    				// Start Task Fail
};
////////////////// Wait Fail  //////////////////
enum wait_err
{
	_WAIT_SEM_FAIL = 0x13000001,    // Wait Semaphor Fail
	_WAIT_MSG_FAIL,   				// Receive Message Fail
	_DELAY_TSK_FAIL,  				// Delay Task Fail
};


///////////////////////////////////////////////////////
// 				Task interface 						//
///////////////////////////////////////////////////////

/* task priority config */

#define LOW_PRI             31    // Min number, but the highest level
#define HIGH_PRI            0     // Max number,but the lowest level
#define DEF_PRI             20    // default level for normal thread
//jeff add,2003-8-19
//增加HSR等级和动态等级定义
#define HSR_PRI             10    // HSR level, higher than normal
#define DYN_PRI             18    // Dynamic level, only  once.
//jeff add,2003-8-19
typedef struct t_ctsk
{
    FP      task;
    PRI     itskpri;
    INT32   stksz;
    UINT32  quantum;
    UINT32  para1;
    UINT32  para2;
    char    name[3];    //added by jeff wu 2003-05-27
} T_CTSK;

#define T_CTHD	T_CTSK

typedef TMO                DLYTIME;        /* delay time */

//
//
//
//------------Standard TDS Interface Specification-------------------
//
//					Version 3.0
//								Nov 5, Jeff Wu


///////////////////////////////////////////////////////
//		Part I	 Thread Interface
//////////////////////////////////////////////////////
ER OS_EnableDispatch(void);
ER OS_DisableDispatch(void);
ID_THREAD OS_CreateThread(T_CTHD *);

//ER OS_StartThread(ID_THREAD thdid);

ER OS_ThreadSleep(DLYTIME dlytime);

void OS_Exit(ER ExitCode);


ER OS_DeleteThread(ID_THREAD);

ID OS_GetCurrentThreadID(void);

#define OS_ThreadSleep	dly_tsk
#define OS_TaskSleep OS_ThreadSleep


///////////////////////////////////////////////////////
//		Part II		memory pool interface  				//
///////////////////////////////////////////////////////

#define TA_TPRI            0x00000001
#define TTW_MPL            0x00001000
#define ref_mpl            i_ref_mpl

typedef	struct t_cmpl
{
	VP	exinf;				// extended information
	ATR	mplatr;
	INT	mplsz;				// size of request
} T_CMPL;

typedef struct t_rmpl
{
	BOOL_ID	wtsk;
	INT	frsz;
	INT	maxsz;
} T_RMPL;



// Common Interface
		VP OS_MemAlloc(INT blksz);
		ER OS_MemFree(VP blk);

//Special Interface
//for un-cached memory allocation & free
VP OS_MemAllocSyn(INT blksz);
ER OS_MemFreeSyn(VP blk);




///////////////////////////////////////////////////////
// 		Part III		Event flags interface 				//
///////////////////////////////////////////////////////

#define TA_TFIFO           0          /* FIFO wait queue */
#define TA_WSGL            0x00000000
#define TA_WMUL            0x00000008

#define TWF_ANDW           0x00000001
#define TWF_ORW            0x00000002
#define TWF_CLR            0x00000004

// Mask to clear all flag
#define TDS_FLAG_ALL       0xFFFFFFFF

// Common Interface
ID OS_CreateFlag(UINT32 flgptn);
ER OS_DelFlag(const ID flgid);
ER OS_SetFlag(ID, UINT);
ER OS_ClearFlag(ID, UINT);
ER OS_WaitFlagTimeOut(UINT *, ID, UINT, UINT, TMO);


// Special Interface
#define	OS_WaitFlag(A,B,C,D)		OS_WaitFlagTimeOut(A,B,C,D,TMO_FEVR)
#define	OS_TestFlag(A,B,C,D)		OS_WaitFlagTimeOut(A,B,C,D,TMO_POL)


///////////////////////////////////////////////////////
//		Part IV	 Semaphore interface 					//
//////////////////////////////////////////////////////

// Common Interfaces
ID OS_CreateSemaphore(INT32 semcnt);
ER OS_DelSemaphore(ID);
ER OS_FreeSemaphore(ID);
ER OS_AcquireSemaphoreTimeOut(ID semid, TMO tmout);


//Special Interface
#define OS_AcquireSemaphore(A)		OS_AcquireSemaphoreTimeOut(A,TMO_FEVR)


///////////////////////////////////////////////////////
//		Part V		Message buffer interface 			//
///////////////////////////////////////////////////////
typedef enum{
	MSG_TYPE_INPUT,
	MSG_TYPE_SIGNAL,
	MSG_TYPE_DEVSTA,
	MSG_TYPE_NVINFO,
	MSG_TYPE_PAUSE,
	MSG_TYPE_RESUME,
	MSG_TYPE_UPDATE,
	MSG_TYPE_CLOSEWIN,//add by sunny 0818
	MSG_TYPE_REDRAWWIN,//add by sunny 0818
	MSG_TYPE_HSR,			//add by jeff 0820
	MSG_TYPE_EXIT,//add by sunny 1029
	MSG_TYPE_SYSTEM,	// Singer for System Message
	MSG_TYPE_ATAPI_INFO,	// tom for atapi tray-closed message
}MSG_TYPE;


typedef struct t_cmbf
{
    INT     bufsz;
    INT     maxmsz;
    char    name[3];	//added by jeff wu 2003-05-27
} T_CMBF;

typedef struct tagMSG_HEAD
{
	MSG_TYPE	bMsgType;		// Message type
	ID_MODULE	bModuleID;		// Module ID of the sender
}MSG_HEAD, *PMSG_HEAD;


typedef struct
{
	ID_MODULE   ModID;			//sender module ID
	MSG_TYPE    MsgType;		//sender message Type
	DWORD       dPara;			// info 1
	DWORD       wPara;			// info 2,should be dPara2
	WORD        wID;			//ext ID info
	TMO         SysTime;		//current time,ms based
}TMSG,*LPTMSG;

// Common Interface
ID OS_CreateMsgBuffer (T_CMBF *pk_cmbf);
ER OS_SendMsgTimeOut(ID, VP, INT, TMO);
ER OS_GetMsgTimeOut(VP, INT *, ID, TMO);
ER OS_DelMessageBuffer(ID);


//Special Interface
ER OS_GetMessage(VP, INT *, ID);
ER OS_PeekMessage(VP, INT *, ID);
ER OS_SendMessage(ID, VP, INT);
ER OS_PostMessage(ID, VP, INT);



///////////////////////////////////////////////////////
//		Part VI		Timer interface 					//
///////////////////////////////////////////////////////

// Timer Type
#define TIMER_ALARM         0x01
#define TIMER_CYCLIC        0x02

#define TMO_POL             0
#define TMO_FEVR            (DWORD)(-1)

typedef void                (*TMR_PROC)(UINT);

typedef struct t_timer
{
    TMR_PROC      		callback;
    UINT16   			type;
    TMO				time;
    UINT				param;
    char				name[3];	//added by jeff wu 2003-05-27
} T_TIMER;


// Common Interface
DWORD OS_GetTickCount(void);
void OS_Delay(WORD us);
ID OS_CreateTimer (T_TIMER* pTimerInfo);
ER OS_DeleteTimer(ID);
ER OS_SetTimer(ID,UINT32);
ER OS_ActivateTimer(ID,BOOL);


//Special Interface

#define TCY_OFF         0x00000000          /* Not use */
#define TCY_ON          0x00000001
#define TCY_INI         0x00000002          /* Not use */

#define TTM_ABS         0x00000000
#define TTM_REL         0x00000001

///////////////////////////////////////////////////////
//		Part VII		ISR & HSR Interface				//
///////////////////////////////////////////////////////

//支持中断嵌套
//从Entry.S中移出
#if 0
#define NESTEDIRQ
#else
#define NO_NESTEDIRQ
#endif


typedef void	(*ISR_PROC)(UINT);



ER OS_RegisterHSR(T_HSR_PROC_FUNC_PTR,DWORD);
//BOOL OS_RegisterHSR( T_HSR_PROC_FUNC_PTR pHsrFunc,DWORD para1,DWORD para2,BOOL bDispatch);

ER OS_RegisterISR(UINT16 irq, ISR_PROC irq_rsp, UINT32 Param);
//ER OS_RegisterISR(ID irq, FP irq_rsp);

ER OS_DeleteISR(UINT16 irq, ISR_PROC IsrFunc);

void OS_DisableInterrupt(void);
void OS_EnableInterrupt(void);
//for legacy
unsigned long ref_ie();
#define Enter_UP()	OS_DisableInterrupt()
#define Leave_UP()	OS_EnableInterrupt()
#define T2EnInt()	OS_EnableInterrupt()


///////////////////////////////////////////////////////
//		Part VIII		Cache Interface				//
///////////////////////////////////////////////////////


void OS_InvalidateCache (DWORD dwStartAddr, DWORD dwByteLen);	//???????

//cache declare
extern void dcacheop_HIT_WB_INV(DWORD dwStartAddr, DWORD dwByteLen);
//	Arguments:
//		DWORD dwStartAddr start address to flushed
//		DWORD dwByteLen byte length
//	Return Value:
//		None
#define OS_FlushCache dcacheop_HIT_WB_INV


///////////////////////////////////////////////////////
//  Mutex Interface
///////////////////////////////////////////////////////
ID OS_CreateMutex(void);
ER OS_DeleteMutex(ID meid);
ER OS_LockMutex(ID meid,TMO tmout);
ER OS_UnlockMutex(ID meid);
///////////////////////////////////////////////////////
//  Operation of CP0 counter, Speer, 2003.12.24
///////////////////////////////////////////////////////

//////////// defined in c0_count.s //////////
extern DWORD get_c0_count(void);
extern void set_c0_count(DWORD dwCntSet);
extern void set_c0counter_mode(BYTE mode);
extern BYTE get_c0counter_mode(void);
/////////////////////////////////////////////

#define OS_GetC0Counter       get_c0_count
#define OS_SetC0Counter       set_c0_count
#define OS_SetC0CounterMode   set_c0counter_mode
#define OS_GetCounterMode     get_c0counter_mode


///////////////////////////////////////////////////////
//  		Signal interface  						//
///////////////////////////////////////////////////////
#define SIGNAL_TYPE	0x1		/* used for ID_STRUCTYPE */

// Common Interface
//ER OS_APRegisterSignalNotify(ID_MODULE devid, ID mbfid);
//ER OS_DrvRegisterSignalNotify(ID_MODULE devid, T_SIG_PROC_FUNC_PTR pSigProcFunc);
//ER OS_APUnregisterSignalNotify(ID_MODULE devid, ID mbfid);
//ER OS_DrvUnregisterSignalNotify(ID_MODULE devid, T_SIG_PROC_FUNC_PTR pSigProcFunc);
//ER OS_SendSignal(ID_MODULE devid , ID_SIGNAL sigid, DWORD dwParam1, DWORD dwParam2);
//void CreateSignalThread(void);
///////////////////////////////////////////////////////
//			Device interface						//
///////////////////////////////////////////////////////


//----------Marked by Jeff Wu,2003--9--4-------Unify TMSG---//
// Message type definition
//#define	MSG_TYPE_SIGNAL		1
//#define	MSG_TYPE_GENERAL		2

//typedef  struct  tagDEVTOAP
//{
//	MSG_HEAD 	head_msg;
//	ID_SIGNAL	sigid;
//	DWORD	dwParam1;
//	DWORD	dwParam2;
//}DEVTOAP, *LPDEVTOAP;
//----------Marked by Jeff Wu,2003--9--4-------Unify TMSG---//

///////////////////////////////////////////////////////
//			software interrupt 						//
///////////////////////////////////////////////////////

/***************************************************************/


///////////////////////////////////////////////////////
//				miscellaneous interface				//
///////////////////////////////////////////////////////

void irq_initialize(void);



/******************************************
*	Macro define of Critical Section functions
*	derived from Semaphore Mechanism
*	Detail code in semapho.c
*	Jeff Wu
*			2003 -07-16
******************************************/

#define OS_InitCritical()		 		OS_CreateSemaphore(1)		//create a semaphore count=1
#define OS_EnterCritical(sID) 			OS_AcquireSemaphore((sID))	//rcquire semaphore
#define OS_LeaveCritical(sID)			OS_FreeSemaphore((sID))	//free semaphore
#define OS_DeleteCritical(sID)			OS_DelSemaphore(sID)		//delete semaphore


//Old Define,will be expired
//task.c
void OS_EnterCriticalSection(void);
void OS_LeaveCriticalSection(void);

#ifndef ALi_x86_Module
#define Enter_CriticalSection()	OS_EnterCriticalSection()
#define Leave_CriticalSection()	OS_LeaveCriticalSection()
#define EnterCriticalSection()	OS_EnterCriticalSection()
#define LeaveCriticalSection()	OS_LeaveCriticalSection()
#endif



///////////////////////////////////////////////////////
//			Macro Definition for legacy compability
//////////////////////////////////////////////////////
//#define	CreateTask			OS_CreateTask
//#define	RequestExecTask	OS_RequestExecTask
//#define	DelayTask			OS_TaskSleep
//#define	CreateThread		OS_CreateThread
//#define	StartThread			OS_StartThread
//#define	DelayThread			OS_ThreadSleep
//#define	OS_DelayThread		OS_ThreadSleep
//#define	malloc_ptr			OS_MemAlloc
//#define	free_mpl			OS_MemFree
//#define	cre_flg_id			OS_CreateFlag
//#define	del_flg				OS_DelFlag
//#define	set_flg				OS_SetFlag
//#define	clr_flg				OS_ClearFlag
//#define	wai_flg				OS_WaitFlag
//#define	twai_flg				OS_WaitFlagTimeOut
//#define	pol_flg				OS_TestFlag
//#define	cre_sem_id			OS_CreateSemaphore
//#define	del_sem				OS_DelSemaphore
//#define	sig_sem				OS_FreeSemaphore
//#define	wai_sem			OS_AcquireSemaphore
//#define	twai_sem			OS_AcquireSemaphoreTimeOut
//#define	cre_mbf_id			OS_CreateMsgBuffer
//#define	rcv_mbf				OS_GetMessage
//#define	prcv_mbf			OS_PeekMessage
//#define	snd_mbf			OS_SendMessage
//#define	psnd_mbf			OS_PostMessage
//#define	del_mbf				OS_DelMessageBuffer
//#define	def_alm_id			OS_CreateAlarm
//#define	def_cyc_id			OS_CreateCyclic
//#define	del_cyc				OS_DeleteCyclic
//#define	act_cyc				OS_ActivateCyclic
//#define	RequestHSR			OS_RequestHSR
//#define	ReleaseHSR			OS_ReleaseHSR
//#define	APRegisterSignalNotify	OS_APRegisterSignalNotify
//#define	DrvRegisterSignalNotify	OS_DrvRegisterSignalNotify
//#define	APUnregisterSignalNotify	OS_APUnregisterSignalNotify
//#define	DrvUnregisterSignalNotify	OS_DrvUnregisterSignalNotify
//#define	SendSignal			OS_SendSignal
//#define	RegisterISR			OS_RegisterISR
//#define	OS_DisableInterrupt				OS_DisableInterrupt
//#define	OS_EnableInterrupt				OS_EnableInterrupt
//ER OS_CreateTask(ID_TASK,T_CTSK*);
//ER OS_RequestExecTask(ID_TASK);
//ER OS_StartThread(ID_THREAD);
//ER OS_ThreadSleep(DLYTIME);
//function for Task Dispatch mechanism Enable/Disable


#include <api/libc/printf.h>


#ifdef _DEBUG_RTOS
	#define KER_PRINTF DEBUGPRINTF
#else
	#define KER_PRINTF(...)
#endif


#ifdef __cplusplus
}
#endif










#endif /* _ITRON_H_ */



