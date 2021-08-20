#ifndef  OS_CPU_H_
#define  OS_CPU_H_

typedef unsigned int   OS_CPU_SR;
#define  OS_ENTER_CRITICAL() (void)cpu_sr;
#define  OS_EXIT_CRITICAL()

#endif // OS_CPU_H_
