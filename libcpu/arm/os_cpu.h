#ifndef  OS_CPU_H_
#define  OS_CPU_H_

typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */

//*
//*********************************************************************************************************
//*                                              Cortex-M1
//*                                      Critical Section Management
//*
//* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
//*             will be enabled even if they were disabled before entering the critical section.
//*             NOT IMPLEMENTED
//*
//* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In other words, if
//*             interrupts were disabled before entering the critical section, they will be disabled when
//*             leaving the critical section.
//*             NOT IMPLEMENTED
//*
//* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
//*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
//*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
//*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
//*             into the CPU's status register.
//*********************************************************************************************************
//*

#define  OS_CRITICAL_METHOD   3

#if OS_CRITICAL_METHOD == 3
	#define  OS_ENTER_CRITICAL()  {cpu_sr = OS_CPU_SR_Save();}
	#define  OS_EXIT_CRITICAL()   {OS_CPU_SR_Restore(cpu_sr);}
#endif

#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  OS_CPU_SR_Save(void);
	void       OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
#endif

#endif // OS_CPU_H_
