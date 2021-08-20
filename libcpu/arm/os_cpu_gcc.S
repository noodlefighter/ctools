 
    .cpu    cortex-m3
    .fpu    softvfp
    .syntax unified
    .thumb
    .text

/*
 * OS_CPU_SR OS_CPU_SR_Save(void);
 */
    .global OS_CPU_SR_Save
    .type OS_CPU_SR_Save, %function
OS_CPU_SR_Save:
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR

/*
 * void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
 */
    .global OS_CPU_SR_Restore
    .type OS_CPU_SR_Restore, %function
OS_CPU_SR_Restore:
    MSR     PRIMASK, R0
    BX      LR
