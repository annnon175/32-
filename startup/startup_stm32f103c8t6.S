.syntax unified
.cpu cortex-m3
.thumb

.global _estack
.global Reset_Handler
.extern main
.extern SysTick_Handler

.section .isr_vector,"a",%progbits
.type vector_table, %object
vector_table:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* STM32F103 medium-density 的 43 个外部中断，本实验均使用默认处理。 */
    .rept 43
    .word Default_Handler
    .endr
.size vector_table, .-vector_table

.section .text.Reset_Handler,"ax",%progbits
.thumb_func
Reset_Handler:
    /* 将 Flash 中的 .data 初值复制到 RAM。 */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcc 2f
    b 3f
2:
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b

3:
    /* 将 .bss 清零。 */
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
4:
    cmp r0, r1
    bcc 5f
    b 6f
5:
    str r2, [r0], #4
    b 4b

6:
    bl main
7:
    b 7b
.size Reset_Handler, .-Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.thumb_func
Default_Handler:
    b Default_Handler

.macro weak_handler name
    .weak \name
    .thumb_set \name, Default_Handler
.endm

weak_handler NMI_Handler
weak_handler HardFault_Handler
weak_handler MemManage_Handler
weak_handler BusFault_Handler
weak_handler UsageFault_Handler
weak_handler SVC_Handler
weak_handler DebugMon_Handler
weak_handler PendSV_Handler

