;*******************************************************************************
;   Lab 5b - traffic.asm
;
;   Description:  1. Turn the large green LED and small red LED on and
;                    delay 20 seconds while checking for orange LED.
;                    (If orange LED is on and 10 seconds has expired, immediately
;                    skip to next step.)
;                 2. Turn large green LED off and yellow LED on for 5 seconds.
;                 3. Turn yellow LED off and large red LED on.
;                 4. If orange LED is on, turn small red LED off and small green
;                    LED on.  After 5 seconds, toggle small green LED on and off
;                    for 6 seconds at 1 second intervals.  Finish by toggling
;                    small green LED on and off for 4 seconds at 1/5 second
;                    intervals.
;                    Else, turn large red LED on for 5 seconds.
;                 5. Repeat the stoplight cycle.
;
;   I certify this to be my source code and not obtained from any student, past
;   or current.
;
;*******************************************************************************
;                            MSP430F2274
;                  .-----------------------------.
;            SW1-->|P1.0^                    P2.0|<->LCD_DB0
;            SW2-->|P1.1^                    P2.1|<->LCD_DB1
;            SW3-->|P1.2^                    P2.2|<->LCD_DB2
;            SW4-->|P1.3^                    P2.3|<->LCD_DB3
;       ADXL_INT-->|P1.4                     P2.4|<->LCD_DB4
;        AUX INT-->|P1.5                     P2.5|<->LCD_DB5
;        SERVO_1<--|P1.6 (TA1)               P2.6|<->LCD_DB6
;        SERVO_2<--|P1.7 (TA2)               P2.7|<->LCD_DB7
;                  |                             |
;         LCD_A0<--|P3.0                     P4.0|-->LED_1 (Green)
;        i2c_SDA<->|P3.1 (UCB0SDA)     (TB1) P4.1|-->LED_2 (Orange) / SERVO_3
;        i2c_SCL<--|P3.2 (UCB0SCL)     (TB2) P4.2|-->LED_3 (Yellow) / SERVO_4
;         LCD_RW<--|P3.3                     P4.3|-->LED_4 (Red)
;   TX/LED_5 (G)<--|P3.4 (UCA0TXD)     (TB1) P4.4|-->LCD_BL
;             RX-->|P3.5 (UCA0RXD)     (TB2) P4.5|-->SPEAKER
;           RPOT-->|P3.6 (A6)          (A15) P4.6|-->LED 6 (R)
;           LPOT-->|P3.7 (A7)                P4.7|-->LCD_E
;                  '-----------------------------'
;
;*******************************************************************************
;*******************************************************************************
        #include "msp430.h"      ; needed to make GNU assembler work

       .equ GREEN_DELAY,200
       .equ RED_DELAY,50
       .equ YELLOW_DELAY,50
       .equ TIMER_DELAY,1111               ; experimentally determined constant that gives 1/10 of a second delay, VLO clock runs at 11.1 KHZ 
       .equ STACK_SIZE,0x600
       .equ GREEN_MASK,0x01
       .equ RED_MASK,0x08
       .equ YELLOW_MASK,0x04
       .equ ORANGE_MASK,0x02
       .equ RED2_MASK,0x40
       .equ GREEN2_MASK,0x10
       .equ SW_ALL,0x0f
       .equ PED_DELAY,50
       .equ PED1_FRQ,10
       .equ PED2_FRQ,2
       .equ PED1_DUR,6
       .equ PED2_DUR,20
       .equ GREEN,0x01
       .equ YELLOW,0x02
       .equ RED,0x03
       .equ PED,0x04
       .equ MIN_GREEN_LENGTH,101
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
          bis.b #\mask,&P4DIR 
          xor.b #\mask,&P4OUT
        .endm
       .macro P3LIGHT_TOGGLE mask
          bis.b #\mask,&P3DIR 
          xor.b #\mask,&P3OUT
        .endm
       .macro P3LIGHT_ON mask
          bis.b #\mask,&P3DIR 
          bis.b #\mask,&P3OUT 
       .endm
       .macro P3LIGHT_OFF mask
          bis.b #\mask,&P3DIR 
          bic.b #\mask,&P3OUT
       .endm
       .macro SW_CONFIG mask           ;macro to configure all buttons to generate interrupts
          bic.b  #\mask,&P1SEL          ; select GPIO
          bic.b  #\mask,&P1DIR          ; configure P1.0 as inputs
          bis.b  #\mask,&P1OUT          ; use pull-up
          bis.b  #\mask,&P1REN          ; enable pull-up
          bis.b  #\mask,&P1IES          ; trigger on high to low transition
          bis.b  #\mask,&P1IE           ; P1.0 interrupt enabled
          bic.b  #\mask,&P1IFG          ; P1.0 IFG cleared
       .endm
;------------------------------------------------------------------------------
            .data                         ; data segment 
;------------------------------------------------------------------------------
timer_remaining: .word 0x0000
state: .byte 0x01                           ; initial state green


;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
;-------------------------------------------------------------------------------
start:  mov.w   #__stack,SP             ; init stack pointer _stack initialized in linker script
        mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT
        SW_CONFIG SW_ALL                ; configure all buttons to generate interrupts
start_cycle:
	call #green                     ; do green state
	mov.w #YELLOW,&state            ; update state var
	call #yellow                    ; do yellow state
	P4LIGHT_TEST ORANGE_MASK        ; test if orange is on
	jeq do_red                      ; n
	jmp do_ped                      ; y

do_red: mov.w #RED,&state               ; update state var
	call #red                       ; do red state
	mov.w #GREEN,&state             ; update state var
	jmp start_cycle                 ; restart cycle
do_ped: mov.w #PED,&state               ; update state
	call #pedestrian                ; do ped state
	mov.w #GREEN,&state             ; update state 
	jmp start_cycle                 ; restart cycle

green:  P4LIGHT_ON GREEN_MASK           ; turn on green
        P4LIGHT_ON RED2_MASK            ; trun on red LED
        push #GREEN_DELAY               ; pass var
        call #delay                     ; do delay
        add.w #2,SP                     ; clean up stack
        P4LIGHT_OFF GREEN_MASK          ; turn off green
        ret                             ;

red:    P4LIGHT_ON RED_MASK             ; turn on red
        push #RED_DELAY                 ; pass var
        call #delay                     ; do delay
        add.w #2,SP                     ; clean up stack
        P4LIGHT_OFF RED_MASK            ; turn off red
        ret

yellow: P4LIGHT_ON YELLOW_MASK          ; turn on yellow
        push #YELLOW_DELAY              ; pass var
        call #delay                     ; do delay
        add.w #2,SP                     ; clean up stack
        P4LIGHT_OFF YELLOW_MASK         ; turn off yellow
        ret

pedestrian:
	P4LIGHT_ON RED_MASK             ; turn on big red
	P4LIGHT_OFF ORANGE_MASK+RED2_MASK   ; turns off big orange and little red LED
	P3LIGHT_ON GREEN2_MASK          ; turn on little green LED
	push #PED_DELAY                 ; pass var
	call #delay                     ; do delay
	add.w #2,SP                     ; clean stack
	push #PED1_FRQ                  ; pass frequency of flashes (these params will toggle the LED once a second for 6 seconds)
	push #PED1_DUR                  ; pass the number of loop iterations, or the total nunmber of toggles  
	call #flash_green2              ; call subroutine that blinks green LED, takes frequency and duration 
	add.w #4,SP                     ; clean up stack
	push #PED2_FRQ                  ; repeat process with different params 
	push #PED2_DUR                  ; these params will toggle the LED once every 0.2 seconds for 4 seconds
	call #flash_green2              ;
	add.w #4,SP                     ;

	P4LIGHT_OFF RED_MASK            ; turn off big red 
	P3LIGHT_OFF GREEN2_MASK         ; turn off little green
	ret

flash_green2: 
	push r14                        ; save r14
	mov.w 4(SP),r14                 ; set up loop var
flg2_lp:
	P3LIGHT_TOGGLE GREEN2_MASK      ; toggle little greeen
	push 8(SP)                      ; push decrements SP before moving onto the stack so I need 8 instead of 6 to compensate 
	call #delay                     ; do delay
	add.w #2,SP                     ; clean up stack
	sub.w #1,r14                    ; done?
	jne flg2_lp                     ; n
	pop r14                         ; y, restore r14
	ret
                              

delay:  bis.b  #LFXT1S_2, &BCSCTL3     ; select VLO clock
        mov.w #(TASSEL_1|MC_1), &TACTL ; use ACLK of timer A in mode 1 (go up to CCR0)
        mov.w #TIMER_DELAY, &TACCR0 ; set up interrupt frequency (1/10 of a second)
        mov.w #CCIE, &TACCTL0 ; enable timer interrupt
        mov.w 2(SP),&timer_remaining ; store delay length
        bis.w  #(LPM3_bits+GIE),SR    ; enable general interrupts and put CPU to sleep in low power mode 3
        ret

timer_isr: ;this is the timer interrupt service routine
        P4LIGHT_TEST ORANGE_MASK ;test if orange is on (here I implement a bonus feature: if green has been on for 10 seconds and orange is on, jump to yellow state )
        jeq do_dec ; n proceed as normal
        jmp test_ifg ;y check state
test_ifg:
        cmp.w #GREEN,&state ; in green state?
        jeq check_tr       ; y  check how much time is left in green state
        jmp do_dec         ; n proceed as normal
check_tr:
        cmp.w #MIN_GREEN_LENGTH,&timer_remaining ;have at least 10 secs passed? 
        jl end_delay ;y
do_dec: sub.w #1,&timer_remaining
        jne tsr_end
end_delay:
        bic.w   #LPM3_bits,0(SP) ; clears 'sleep' bits of the saved SR so when SR is restored by reti, the CPU will wake up
 tsr_end:
        reti

p1_isr: P4LIGHT_ON ORANGE_MASK         ; turn on orange (this is the button interrupt service routine)
        bic.b  #SW_ALL,&P1IFG          ; clear P1.0 Interrupt Flag
        reti

;-------------------------------------------------------------------------------
;           Stack Pointer definition
;-------------------------------------------------------------------------------

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset","xa"              ; MSP430 RESET Vector (adresses of sections are configured in linker script)
            .word   start                   ; start address
            .sect   ".timer","xa"          ; timer interrupt vector (xa needed for mspdebug to recognize and properly install program)
            .word   timer_isr              ; timer interrupt service routine
            .sect   ".p1_button", "xa"     ; button interrupt service vector (without xa all sections except .text will be ignored)
            .word   p1_isr                 ; button interrupt service routine
            .end
