;*******************************************************************************
;     Project:  morse.asm
;      Author:  Student Code
;
; Description:  Outputs a message in Morse Code using a LED and a transducer
;               (speaker).  The watchdog is configured as an interval timer.
;               The watchdog interrupt service routine (ISR) toggles the green
;               LED every second and pulse width modulates (PWM) the speaker
;               such that a tone is produced.
;
;  numbers--->N0$--->DASH,DASH,DASH,DASH,DASH,END      ; 0
;             N1$--->DOT,DASH,DASH,DASH,DASH,END       ; 1
;             ...
;             N9$--->DASH,DASH,DASH,DASH,DOT,END       ; 9
;
;  letters--->A$---->DOT,DASH,END                      ; A
;             B$---->DASH,DOT,DOT,DOT,END              ; B
;             ...
;             Z$---->DASH,DASH,DOT,DOT,END             ; Z
;
;	Morse code is composed of dashes and dots, or phonetically, "dits" and
;    "dahs".  There is no symbol for a space in Morse, though there are rules
;    when writing them.
;
;	1. One dash is equal to three dots
;	2. The space between parts of the letter is equal to one dot
;	3. The space between two letters is equal to three dots
;	4. The space between two words is equal to seven dots.
;
;	5 WPM = 60 sec / (5 * 50) ELEMENTs = 240 milliseconds per ELEMENT.
;	ELEMENT = (WDT_IPS * 6 / WPM) / 5
;
;******************************************************************************
; System equates --------------------------------------------------------------
            
         #include "msp430.h"            
;needed to make gnu assembler work

/*	.equ myCLOCK,1200000                 ; 1.2 Mhz clock
	.equ WDT_CTL,WDT_MDLY_0_5            ; WD: Timer, SMCLK, 0.5 ms
	.equ WDT_CPI,500                     ; WDT Clocks Per Interrupt (@1 Mhz)
	.equ WDT_IPS,myCLOCK/WDT_CPI         ; WDT Interrupts Per Second*/
	.equ STACK,0x0600                  ; top of stack
        .equ TIMER_DELAY,5               ; experimentally determined constant that gives 0.5ms second delay, VLO clock runs at 11.1 KHZ 


; External references ---------------------------------------------------------
            .global    numbers                 ; codes for 0-9
            .global    letters                 ; codes for A-Z
 
; Morse Code equoates ---------------------------------------------------------
	.equ ELEMENT_T_S,2020        ; element times speed
        .equ DASH_LENGTH,3  ; 
        .equ SPACE_LENGTH,1 ; space is implemented like a character, thus it is preceded and followed by a pause of length 3. Hence it must be 1 since 3+3+1=7
        .equ PAUSE_LENGTH, 3
        .equ NUM0,48
        .equ NUMA,65
	.equ NUMa,97
        .equ NUMZ,91
        .equ NUM9,(NUM0+10)
        .equ NUM_SPACE,32
        .equ RED2_MASK,0x40
	.equ GREEN2_MASK,0x10
	.equ RED_MASK,0x08
	.equ GREEN2_CNT_RESET,2200
	.equ DEBOUNCE,10
        .equ  BUTTON_MASK,0x07
        .macro P3LIGHT_ON mask      ; macros to operate on P3 and P4 lights
            bis.b #\mask,&P3DIR  
           bis.b #\mask,&P3OUT 
        .endm
        .macro P3LIGHT_OFF mask
            bis.b #\mask,&P3DIR  
           bic.b #\mask,&P3OUT
        .endm
       .macro P3LIGHT_TOGGLE mask
           bis.b #\mask,&P3DIR  
          xor.b #\mask,&P3OUT
        .endm
       .macro P4LIGHT_ON mask      ; macros to operate on P4 and P3 lights
          bis.b #\mask,&P4DIR 
          bis.b #\mask,&P4OUT 
       .endm
        .macro P4LIGHT_OFF mask
          bis.b #\mask,&P4DIR 
          bic.b #\mask,&P4OUT
        .endm
       .macro P4LIGHT_TEST mask
          bit.b #\mask,&P4OUT
        .endm
       .macro P4LIGHT_TOGGLE mask
           xor.b #\mask,&P4OUT
	.endm

                            
;--------------------------------------------------------------------------------
.data              ;data segment
;--------------------------------------------------------------------------------
beep_cnt: .word 0x0000              ; beeper flag
delay_cnt: .word 0x0000             ; delay flag
debounce_cnt: .word 10
speaker_on: .word 0x0001
green_toggle_cnt: .word 2200
element: .word 0x0000
dash_element: .word 0x0000
pause_element: .word 0x0000
space_element: .word 0x0000
speed: .byte 5
	.byte 0x00
message:    .string "Hello CS 124 World"                 ; hello word message
            .byte   0
            .align  2                       ; align on word boundary


; Program section -------------------------------------------------------------
            .text                           ; program section
;------------------------------------------------------------------------------

start:  mov.w   #__stack,SP             ; init stack pointer _stack initialized in linker script
	mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT
	bis.b   #0x2f,&P4DIR            ; set P4.5 as output (speaker)
	clr.w   &beep_cnt               ; clear counters
	clr.w   &delay_cnt
	bis.w   #GIE,SR                 ; enable interrupts
	call #setup_button
	call #update_speed

output_msg:
        mov.w   #message,r4             ; get msg pointer
loop:   push r4                         ; pass pointer to start of message
	call #doCharacter              ; output char
	add.w #2,SP                    ; clean up stack
	inc.w r4                      ; increment pointer
	call #doPause                 ; pause
	tst.w @r4                     ; end of string?
	jnz     loop                    ; n,output next char
	jmp output_msg                  ; y, output message again


; beep (r15) ticks subroutine -------------------------------------------------
beep:   P4LIGHT_ON RED2_MASK
	mov.w   r15,&beep_cnt           ; pass beep length
	push r15                        ; call delay
	call #delay
	add.w #2,SP
 	P4LIGHT_OFF RED2_MASK 
	ret                             ; y


doDot:  mov.w &element,r15              ; pass beep length
	call #beep
	mov.w &element,r15              ; pause after beep
	push r15
	call #delay
	add.w #2,SP                   ; clean up stack
	ret

doDash: mov.w &dash_element,r15      ; pass beep length, equal to 3 times dot length
	call #beep
	mov.w &element,r15           ; short pause after dash
	push r15
	call #delay
	add.w #2,SP 
	ret

doSpace: 
	mov.w &space_element,r15      ; pass delay length, equal to 7 times a dot 
	push r15
	call #delay
	add.w #2,SP
	ret
doPause:
	mov.w &pause_element,r15      ; delay length is three times the length of a dot
	push r15
	call #delay
	add.w #2,SP
	ret 

doCharacter: 
	push r13                  ; save registers (calee-save protocol)
	push r11
	mov.w 6(SP),r11           ; due to the registers pushed on the stack, the argument is now at 6(SP)
	mov.b @r11,r13            ; dereference pointer
	cmp.w #'A',r13           ; is the char less than A?
	jl tst_num                ; y? test if number
	cmp.w #'a',r13          ; Uppercase?
	jge cast_to_upper        ; y? fix it
	jmp do_letter            ; n? proceed as normal
cast_to_upper:
	sub.w #32,r13            ; cast to upper
do_letter:
	cmp.w #('Z'+1),r13          ; is it between A and Z?
	jge done                 ; n, invalid character, return
	sub.w #'A',r13          ; get offset from A
	mov.w #letters,r11       
	add.w r13,r13           ; need to multiply offset by 2 because addresses are words, not bytes
	add.w r11,r13           ; get pointer to address of character
	mov.w @r13,r13          ; dereference first pointer 
	jmp output_char
tst_num:
        cmp.w #'0',r13        ; greater than or equal to 0?
	jl tst_space
	cmp.w #('9'+1),r13       ; less than or equal to 9? if yes, char is a number
	jge done
	sub.w #'0',r13       ; get offset
	mov.w #numbers,r11
	add.w r13,r13           ; need to multiply offset by 2 because addresses are words, not bytes
	add.w r11,r13           ; get pointer to address of character
	mov.w @r13,r13          ; dereference first pointer 
	jmp output_char
tst_space:
	cmp.w #NUM_SPACE,r13    ; is char a space?
	jne done               ; n, done
	call #doSpace         ; y, delay for 1 times the length of a dot
	jmp done
do_dot: call #doDot
        jmp output_char  ; go through 'loop' again
do_dash:call #doDash
        jmp output_char             
output_char: 
	mov.b @r13+,r11    ; get morse code and increment pointer
	cmp.w #END,r11      ; what follows is the assembly equivalent of a switch
	jeq done
	cmp.w #DOT,r11
	jeq do_dot
	cmp.w #DASH,r11
	jeq do_dash
done:   pop r11      ; restore registers and return 
        pop r13
        ret              

setup_button:            
	bic.b  #BUTTON_MASK,&P1SEL          ; select GPIO
	bic.b  #BUTTON_MASK,&P1DIR          ; configure P1.0 as inputs
	bis.b  #BUTTON_MASK,&P1OUT          ; use pull-up
	bis.b  #BUTTON_MASK,&P1REN          ; enable pull-up
	bis.b  #BUTTON_MASK,&P1IES          ; trigger on high to low transition
	bis.b  #BUTTON_MASK,&P1IE           ; P1.0 interrupt enabled
	bic.b  #BUTTON_MASK,&P1IFG          ; P1.0 IFG cleared
	ret

increase_output:
	cmp.b #15,&speed
	jeq inc_done
	add.b #1,&speed
	call #update_speed
inc_done:
	ret

decrease_output: 
	cmp.b #1,&speed
	jeq d_out_done
	sub.b #1,&speed
	call #update_speed
d_out_done:
	ret

update_speed:
	bic.b #0x0f,&P4OUT
	bis.b &speed,&P4OUT
	push r6
	push r4
	push r5
	clr.w r4
	mov.w &speed,r6
	mov.w #ELEMENT_T_S,r5
	call #DIVU
	push r5
	call #update_element
	add.w #2,SP
	pop r5
	pop r4
	pop r6
	ret

update_element: 
	push r10
	push r4
	push r5
	mov.w 8(SP),r10
	mov.w r10,&element
	mov.w r10,&space_element
	mov.w #DASH_LENGTH,r4
	mov.w r10,r5
	call #MPYU
	mov.w r7,&dash_element
	mov.w #PAUSE_LENGTH,r4
	mov.w r10,r5
	call #MPYU
	mov.w r7,&pause_element
	pop r10
	pop r4
	pop r5
	ret
	

; delay (r15) ticks subroutine ------------------------------------------------
delay:  bis.b  #LFXT1S_2, &BCSCTL3     ; select VLO clock
        mov.w #(TASSEL_1|MC_1), &TACTL ; use ACLK of timer A in mode 1 (go up to CCR0)
        mov.w #TIMER_DELAY, &TACCR0 ; set up interrupt frequency (1/10 of a second)
        mov.w #CCIE, &TACCTL0 ; enable timer interrupt
        mov.w 2(SP),&delay_cnt ; store delay length
        bis.w  #(LPM3_bits+GIE),SR    ; enable general interrupts and put CPU to sleep in low power mode 3
        ret


; VLO Timer ISR ----------------------------------------------------------



timer_isr:
	tst.w &green_toggle_cnt
	jeq reset_green_cnt
	dec.w &green_toggle_cnt
	jmp tsr_05
reset_green_cnt:
	P3LIGHT_TOGGLE GREEN2_MASK
	mov.w #GREEN2_CNT_RESET,&green_toggle_cnt

tsr_05: tst.w   &debounce_cnt          ; debouncing?
        jeq   tsr_03                ; n

; debounce switches & process

	dec.w   &debounce_cnt          ; y, decrement count, done?
	jne   tsr_03                ; n
	push    r15                   ; y
	push r14
	mov.b   &P1IN,r15             ; read switches
	and.b   #0x0f,r15
	xor.b   #0x0f,r15             ; any switches?
	jeq   tsr_04                ; n

; process switch inputs (r15 = switches)

tsr_04:	mov.w r15,r14
	and.b #0x01,r14
	jnz toggle_speaker
	jmp check_sw2
toggle_speaker:
	xor.b #0x01,&speaker_on
	jmp p1_done
do_sw2:	call #decrease_output
	jmp p1_done
do_sw3: call #increase_output
	jmp p1_done
check_sw2:
	mov.w r15,r14
	and.b #0x02,r14
	jnz do_sw2
	mov.w r15,r14
	and.b #0x04,r14
	jnz do_sw3
p1_done:pop r14
	pop r15
tsr_03:	tst.w   &beep_cnt               ; beep on?
	jeq   tsr_02                  ; n
	cmp.w #0,&speaker_on
	jeq tsr_02
	dec.w   &beep_cnt               ; y, decrement count
	xor.b   #0x20,&P4OUT            ; beep using 50% PW
tsr_02: tst.w   &delay_cnt              ; delay?
	jeq   end_delay                  ; n
	dec.w   &delay_cnt              ; y, decrement count
	jmp tsr_10
end_delay:
        bic.w   #LPM3_bits,0(SP) ; clears 'sleep' bits of the saved SR so when SR is restored by reti, the CPU will wake up
tsr_10: reti                            ; return from interrupt

p1_isr: 
	bic.b   #0x0f,&P1IFG          ; acknowledge (put hands down)
        mov.w   #DEBOUNCE,&debounce_cnt ; reset debounce count
	reti

; Interrupt Vectors -----------------------------------------------------------
            .sect   ".timer","xa"                ; Watchdog Vector
            .word   timer_isr                 ; Watchdog ISR

            .sect   ".reset" ,"xa"               ; PUC Vector
            .word   start                   ; RESET ISR
            .sect   ".p1_button","xa"
            .word   p1_isr

            .end