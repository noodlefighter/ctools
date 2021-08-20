
    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

;/*
; * OS_CPU_SR OS_CPU_SR_Save(void);
; */
OS_CPU_SR_Save      PROC
    EXPORT  OS_CPU_SR_Save
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR
    ENDP

;/*
; * void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
; */
OS_CPU_SR_Restore    PROC
    EXPORT  OS_CPU_SR_Restore
    MSR     PRIMASK, R0
    BX      LR
    ENDP

    END
