;*******************************************************************************
;   CS 124 Lab 3 - blinky.asm: Software Toggle P1.0
;
;   Description: Toggle P1.0 by xor'ing P1.0 inside of a software loop.
;    Author: Benjamin Pachev.
;    Disclaimer: This assignment is my own work.
;    MCLK = 100*(11+3*34950)/10 = 1048610 cycles per second
;    CPI =  100*(11+3*34950)/100*(6+2*34950)=1.5000 cycles per instruction
;    MIPS = 1048610/1.5000 = 699060 instructions per second.
;      
;             MSP430G5223
;             -----------
;            |       P1.0|-->LED1-RED LED
;            |       P1.3|<--S2
;            |       P1.6|-->LED2-GREEN LED

;
;*******************************************************************************
;           .cdecls C,"msp430.h"             ; MSP430
            #include "msp430.h"
;------------------------------------------------------------------------------
            .equ DELAY_CYCLE , 34950
            .equ NUM_CYCLES , 100
            .text                           ; beginning of executable code
;------------------------------------------------------------------------------
RESET:      mov.w   #0x0280,SP              ; init stack pointer, 2 cycles
            mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT, 5 cycles
            bis.b   #0x01,&P1DIR            ; set P1.6 as output, 4 cycles
            and.b   #0x00, &P1OUT           ; Start with light off, 4 cycles

mainloop:   xor.b   #0x01,&P1OUT        ; turn light on, 4 cycles
            call #delay                 ; leave light on for short time, 5 cycles
            xor.b   #0x01,&P1OUT        ; turn light off, 4 cycles

            mov.w  #NUM_CYCLES,r14      ; init outer loop var, 2 cycles
outerdelayloop: 
            call #delay                 ; call subroutine, 5 cycles
            sub.w #1,r14                ; 1 cycle
            jnz outerdelayloop          ; 2 cycles
            jmp     mainloop            ; start the process over again, 2 cycles
                
delay:      mov.w #DELAY_CYCLE,r15      ; init inner loop var, 2 cycles
delayloop:  sub.w   #1,r15              ; decrement, 1 cycle
            jnz     delayloop           ; 2 cycles
            ret                         ; return, 3 cycles

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   RESET                   ; start address
            .end
