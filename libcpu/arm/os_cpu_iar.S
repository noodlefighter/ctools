
    SECTION    .text:CODE(2)
    THUMB
    REQUIRE8
    PRESERVE8

;/*
; * OS_CPU_SR OS_CPU_SR_Save(void);
; */
    EXPORT OS_CPU_SR_Save
OS_CPU_SR_Save:
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR

;/*
; * void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
; */
    EXPORT  OS_CPU_SR_Restore
OS_CPU_SR_Restore:
    MSR     PRIMASK, R0
    BX      LR

    END
