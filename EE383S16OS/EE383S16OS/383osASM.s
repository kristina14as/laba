;****************** 303osASM.s ***************
;
; EE383 S16
; James Lumpp
; 4/11/2016
;
; Useful declarations

GPIO_PORTF_ICR_R        EQU   0x4002541C  ; interrupt flag for PortF
PF2 	EQU 0x40025010


	
SWITCH_COUNT EQU 0  ; Number of SYstick interrupts before a context switch
INTERRUPT_LR EQU 0xfffffff9  ; Number of SYstick interrupts before a context switch
    THUMB
	
UART0_FR_R		EQU 0x4000C018
UART_FR_RXFE	EQU 0x00000010
UART0_DR_R      EQU 0x4000C000
	
	AREA DATA, ALIGN=2
		; Global variables go here

COUNT SPACE 4
INT_COUNT SPACE 4


	EXTERN EPOCH_SECONDS
	EXTERN Schedule
		
	ALIGN ; make sure the end of this section is aligned

	AREA |.text|, CODE, READONLY, ALIGN=2
		
	EXPORT GPIOPortF_Handler
	EXPORT SysTick_Handler
	EXPORT StartNewTask
	EXPORT COUNT
	IMPORT handleETX
		
GPIOPortF_Handler
	 ; This isr will context switch every SWITCH_COUNT ticks
	 ldr r0, =GPIO_PORTF_ICR_R
	 mov r1, #0x10
	 str r1,[r0]    ; acknowledge flag4
	 ldr r0,=INT_COUNT
	 ldr r1,[r0]
	 subs r1,r1,#1
	 blo context_sw ; perform context switch
	 str r1,[r0]
	 bx lr		    ; return from ISR
SysTick_Handler
	; This isr will context switch every SWITCH_COUNT ticks
	; the 4 lines below cause the blue led to toggle for part 1
	;LDR R1, =PF2
	;LDR R0, [R1]
	;EOR R0, R0, #0x04
	;STR R0, [R1]
	
	LDR R4, = COUNT
	LDR R5, [R4]
	ADD R5, R5, #1
	CMP R5, #10
	BEQ reset

etx_test
	LDR R0, =UART0_DR_R
	LDR R1, [R0]
	CMP R1, #0x03
	BEQ handleETX ;if the character in the RXFE is \c ETF code
	B continue
check_etx
	LDR R0, =UART0_FR_R
	LDR R1, [R0]
	LDR R0, =UART_FR_RXFE
	LDR R2, [R0]
	AND R0, R1, R2
	CMP R0, #0		;if there is no character in the RXFE
	BEQ etx_test
continue
	 STR R5, [R4]
	 ldr r0, =GPIO_PORTF_ICR_R
	 mov r1, #0x10
	 str r1,[r0]    ; acknowledge flag4
	 ldr r0,=INT_COUNT
	 ldr r1,[r0]
	 subs r1,r1,#1
	 blo context_sw ; perform context switch
	 str r1,[r0]
	 bx lr		    ; return from ISR
	 
reset
	LDR R2, =EPOCH_SECONDS
	LDR R3, [R2]
	ADD R3, R3, #1
	STR R3, [R2]
	MOV R5, #0
	B check_etx

context_sw
	 mov r1,#SWITCH_COUNT
	 str r1,[r0]    ;reset INT_COUNT
	 push{r4-r11}   ; save rest of state of the task swithcing out
	 mov r0,sp
	 bl Schedule  ; will call scheduler to select new task
	 mov sp,r0          ; load new tasks sp
	 pop {r4-r11}
	 ldr lr,=INTERRUPT_LR
	 bx lr              ; context switch!

	ALIGN
		
StartNewTask
	mov sp,r0          ; stack top for this new task
	mov r2,#0x01000000  
	push {r2}           ; PSR (mark as thumb)
	push {r1}			; PC start address of the task
	ldr  r1, =task_exit   
	push {r1}     ; LR (if task ever returns)
	mov  r1,#0 ; don't care value of 0 for the other regs
	push {r1} ; r12
	push {r1} ; r3 could be arg to func
	push {r1} ; r2 could be arg to func
	push {r1} ; r1 could be arg to func
	push {r1} ; r0 could be arg to func
	ldr lr,=INTERRUPT_LR
	bx lr              ; context switch to this new tas
	
task_exit
	b	task_exit ; if a task ever returns it 
	              ; gets stuck here for debugging
	
	ALIGN ; make sure the end of this section is aligned (For the code/data to follow)
	END ; mark end of file
