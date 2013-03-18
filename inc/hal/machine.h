/*
 * machine.h
 * This file contains all the hardware dependencies.
 *
 */

#include <sys_config.h>

#define INT_IP2_MASK     0x00000400      /* IP(2) IR */
#define INT_IP3_MASK     0x00000800      /* IP(3) Decoder(M3323) */
#define INT_IP4_MASK     0x00001000      /* IP(4) Servo(M3355), SCI(M3325) */
#define INT_IP5_MASK     0x00002000      /* IP(5) CSCI */
#define INT_IP6_MASK     0x00004000      /* IP(6) DMA(M3355), Watch-dog(M3325) */
#define INT_IP7_MASK     0x00008000      /* IP(7) Timer */

#define INT_BASE      0x80000180		      // INT_BASE is the general Exception Handler Entry when BEV='0'
#define TIMER_COMPARE (SYS_CPU_CLOCK / 2000)  // 1ms @ 1/2 108MHz clock,for ALi-M6311 CPU version
                                              // (on FPGA is 10.368MHz,51840 is 10ms)
#define WDT_MODE        0x08		     	  // Disable Watchdog. -- Justin Wu
                                              // WD timer PCLK/32 3.375MHz. -- Justin Wu
#define WDT_COMPARE   (0xffffffff - (SYS_CPU_CLOCK >> 8)) // watchdog count: 128mS @ 3.375MHz. -- Justin Wu

#define CNT_INTERVAL    3375
#define ALIGN           0x7             // For align 3 2^3 = 8
#define DisSR           0x14000000      // Register SR value, disable interrupt
#define EnSR            0x1400ff01      // Register SR value, enable interrupt
#define OFF_CUR_SLICE	28				//offset of TCB.curslice
#define OFF_SP_END      24              // Offset of TCB.stackend
#define OFF_SP_BGN      20              // Offset of TCB.stackbegin
#define OFF_SP          16              // Offset of TCB.sp
#define OFF_SLICE       10              // Offset of TCB.timeslice ( NOTE: In hi
					// tachi, it's 10 )
#define OFF_ENTRY       12              // Offset of TCB.task
#define SR_IEC          0x00000001
#define SR_EXL          0x00000002      /* Exception Level: 0-normal/1-exception */
#define EXCCODE_MASK    0x0000007c
#define EXCCODE_SYSCALL 0x00000020
#define SYSCALL_0       0x0000000c
#define SYSCALL_1       0x0000004c
#define TCNTADDR        0xff100000
#define TCSRADDR        0xff100004
#define GPIOBASE        0xff101000
#define ADDRBOUND       0xffffffff

// Add for TDS2 code
#define ICACHELINE_SIZE			32	//Bytes per line
#define DCACHELINE_SIZE			16	//Bytes per line
#define ICACHELINE_NUM			127	//put the instruction cache line number here
#define DCACHELINE_NUM			255	//put the data cache line number here

#define POS(x)	((x) * 4)
#define OFS_AT	POS(1)
#define OFS_V0	POS(2)
#define OFS_V1	POS(3)
#define OFS_A0	POS(4)
#define OFS_A1	POS(5)
#define OFS_A2	POS(6)
#define OFS_A3	POS(7)
#define OFS_T0	POS(8)
#define OFS_T1	POS(9)
#define OFS_T2	POS(10)
#define OFS_T3	POS(11)
#define OFS_T4	POS(12)
#define OFS_T5	POS(13)
#define OFS_T6	POS(14)
#define OFS_T7	POS(15)
#define OFS_S0	POS(16)
#define OFS_S1	POS(17)
#define OFS_S2	POS(18)
#define OFS_S3	POS(19)
#define OFS_S4	POS(20)
#define OFS_S5	POS(21)
#define OFS_S6	POS(22)
#define OFS_S7	POS(23)
#define OFS_T8	POS(24)
#define OFS_T9	POS(25)
#define OFS_K0	POS(26)
#define OFS_K1	POS(27)
#define OFS_GP	POS(28)
#define OFS_FP	POS(29)
#define OFS_HI	POS(30)
#define OFS_LO	POS(31)


#define SAVE_SOME		\
		subu	sp, 128;	\
		.set 	noat;		\
		sw	$1, OFS_AT(sp);	\
		sw	v0, OFS_V0(sp);	\
		sw	v1, OFS_V1(sp);	\
		sw	a0, OFS_A0(sp);	\
		sw	a1, OFS_A1(sp);	\
		sw	a2, OFS_A2(sp);	\
		sw      a3, OFS_A3(sp);	\
		sw      t0, OFS_T0(sp);	\
		sw      t1, OFS_T1(sp);	\
		sw      t2, OFS_T2(sp);	\
		sw      t3, OFS_T3(sp);	\
		sw      t4, OFS_T4(sp);	\
		sw      t5, OFS_T5(sp);	\
		sw      t6, OFS_T6(sp);	\
		sw      t7, OFS_T7(sp);	\
		sw      s0, OFS_S0(sp);	\
		sw      s1, OFS_S1(sp);	\
		sw      s2, OFS_S2(sp);	\
		sw      s3, OFS_S3(sp);	\
		sw      s4, OFS_S4(sp);	\
		sw      s5, OFS_S5(sp);	\
		sw      s6, OFS_S6(sp);	\
		sw      s7, OFS_S7(sp);	\
		sw      t8, OFS_T8(sp);	\
		sw      t9, OFS_T9(sp);	\
		sw	k0, OFS_K0(sp);	\
		sw	k1, OFS_K1(sp);	\
		sw      gp, OFS_GP(sp);	\
		sw      fp, OFS_FP(sp);	\
		mfhi	k0;		\
		sw	k0, OFS_HI(sp);	\
		mflo	k0;		\
		sw	k0, OFS_LO(sp);	\
		.set    at;		\
		nop

#define	LOAD_SOME		\
		.set 	noat;		\
		lw	k0, OFS_HI(sp);	\
		mthi	k0;		\
		lw	k0, OFS_LO(sp);	\
		mtlo	k0;		\
		lw	$1, OFS_AT(sp);	\
		lw	v0, OFS_V0(sp);	\
		lw	v1, OFS_V1(sp);	\
		lw	a0, OFS_A0(sp);	\
		lw	a1, OFS_A1(sp);	\
		lw	a2, OFS_A2(sp);	\
		lw      a3, OFS_A3(sp);	\
		lw      t0, OFS_T0(sp);	\
		lw      t1, OFS_T1(sp);	\
		lw      t2, OFS_T2(sp);	\
		lw      t3, OFS_T3(sp);	\
		lw      t4, OFS_T4(sp);	\
		lw      t5, OFS_T5(sp);	\
		lw      t6, OFS_T6(sp);	\
		lw      t7, OFS_T7(sp);	\
		lw      s0, OFS_S0(sp);	\
		lw      s1, OFS_S1(sp);	\
		lw      s2, OFS_S2(sp);	\
		lw      s3, OFS_S3(sp);	\
		lw      s4, OFS_S4(sp);	\
		lw      s5, OFS_S5(sp);	\
		lw      s6, OFS_S6(sp);	\
		lw      s7, OFS_S7(sp);	\
		lw      t8, OFS_T8(sp);	\
		lw      t9, OFS_T9(sp);	\
		lw	k0, OFS_K0(sp);	\
		lw	k1, OFS_K1(sp);	\
		lw      gp, OFS_GP(sp);	\
		lw      fp, OFS_FP(sp);	\
		addu	sp, 128;	\
		.set    at;		\
		nop

  #define SAVE_PARA   \
		.set noat;	\
		subu sp, 8;	\
		sw a0, 4(sp);	\
		sw a1, 0(sp); 	\
		.set at;	\
		nop

  #define LOAD_PARA	\
		.set noat;	\
		lw	a1, 0(sp);	\
		lw	a0, 4(sp);	\
		addu	sp,8;	\
		.set at;	\
		nop



//################################################
#define SAVE_RA			\
		subu		sp,	4;	\
		sw		ra,	0(sp); \
		nop

#define LOAD_RA			\
		lw		ra,	0(sp);	\
		addu	sp,	4; \
		nop

#define SAVE_EPC		\
		subu		sp,	4;	\
		mfc0	k0,	C0_EPC;	\
		nop;			\
		sw		k0,	0(sp); \
		nop
#define LOAD_EPC		\
		lw	k0,	0(sp);	\
		mtc0	k0,	C0_EPC;	\
		nop;			\
		addu	sp,	4; \
		nop

#define PUSH(x)			\
		subu		sp,	4;	\
		sw		x,	(sp); \
		nop

#define POP(x)			\
		lw	x,	(sp);	\
		addu	sp,	4; \
		nop


/*
 * Macros to access the system control coprocessor
 */
	#define STR(x) #x
#define read_32bit_gpr_register(source) \
	({ int __res;								\
	        __asm__ __volatile__(					\
		".set\tpush\n\t"						\
		".set\treorder\n\t"						\
	        "move\t%0,"STR(source)"\n\t"			\
		".set\tpop"							\
	        : "=r" (__res));                                        \
	        __res;})


	#define read_32bit_cp0_register(source)	\
	({ int __res;								\
	        __asm__ __volatile__(					\
		".set\tpush\n\t"						\
		".set\treorder\n\t"						\
	        "mfc0\t%0,"STR(source)"\n\t"			\
		".set\tpop"							\
	        : "=r" (__res));                                        \
	        __res;})

	#define write_32bit_cp0_register(register,value)	\
	        __asm__ __volatile__(						\
	        "mtc0\t%0,"STR(register)"\n\t"				\
		"nop"									\
	        : : "r" (value));

#ifdef __mips16
#define SDBBP()		asm volatile(".half 0xe801; \
								  nop")
#else
#define SDBBP()		asm volatile(".word	0x7000003f; \
								  nop");
#endif

/* Macros for power save */
#if (SYS_CHIP_MODULE == ALI_M3327)
#define DEV_NIM					0x00000000
#define DEV_TVE					0x04000000
#define DEV_DMX1				0x02000000
#define DEV_DMX0				0x01000000
#define DEV_PCI					0x00800000
#define DEV_UART1				0x00400000
#define DEV_UART0				0x00200000
#define DEV_SCR1				0x00100000
#define DEV_SCR0				0x00080000
#define DEV_VE					0x00040000
#define DEV_IDE					0x00020000
#define DEV_CI					0x00010000
#define DEV_SCB					0x00000000
#elif (SYS_CHIP_MODULE == ALI_M3327C)
#define DEV_NIM					0x08000000
#define DEV_TVE					0x04000000
#define DEV_DMX1				0x00000000
#define DEV_DMX0				0x01000000
#define DEV_PCI					0x00000000
#define DEV_UART1				0x00000000
#define DEV_UART0				0x00200000
#define DEV_SCR1				0x00000000
#define DEV_SCR0				0x00000000
#define DEV_VE					0x00040000
#define DEV_IDE					0x00000000
#define DEV_CI					0x00000000
#define DEV_SCB					0x00010000
#elif (SYS_CHIP_MODULE == ALI_S3601)
#define DEV_NIM					0x00000000
#define DEV_TVE					0x00000000
#define DEV_DMX1				0x00000000
#define DEV_DMX0				0x00000000
#define DEV_PCI					0x00000000
#define DEV_UART1				0x00000000
#define DEV_UART0				0x00000000
#define DEV_SCR1				0x00000000
#define DEV_SCR0				0x00000000
#define DEV_VE					0x00000000
#define DEV_IDE					0x00000000
#define DEV_CI					0x00000000
#define DEV_SCB					0x00000000
#else
#define DEV_NIM					0x00000000
#define DEV_TVE					0x00000000
#define DEV_DMX1				0x00000000
#define DEV_DMX0				0x00000000
#define DEV_PCI					0x00000000
#define DEV_UART1				0x00000000
#define DEV_UART0				0x00000000
#define DEV_SCR1				0x00000000
#define DEV_SCR0				0x00000000
#define DEV_VE					0x00000000
#define DEV_IDE					0x00000000
#define DEV_CI					0x00000000
#define DEV_SCB					0x00000000
#endif

/* Device list for all device enter power save */
#define DEV_LEVEL_0				(DEV_NIM|DEV_TVE|DEV_DMX1|DEV_DMX0|DEV_PCI|DEV_UART1|DEV_UART0|DEV_SCR1|DEV_SCR0|DEV_VE|DEV_IDE|DEV_CI|DEV_SCB)
/* Device list for IR, panel and two UART work only power save */
#define DEV_LEVEL_1				(DEV_NIM|DEV_TVE|DEV_DMX1|DEV_DMX0|DEV_PCI|DEV_SCR1|DEV_SCR0|DEV_VE|DEV_IDE|DEV_CI|DEV_SCB)
/* Device list for IR, panel, UART, SCB and stream input work only power save */
#define DEV_LEVEL_2				(DEV_TVE|DEV_DMX0|DEV_UART1|DEV_UART0|DEV_VE|DEV_IDE|DEV_CI)

// Add for dual-CPU chip
#define CPU_MASTER_SLAVE_FLAG	0x400	// bit 10, 1:master cpu, 0: slave cpu
