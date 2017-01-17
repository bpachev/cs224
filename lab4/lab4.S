#include "msp430.h"
MOV.W   #0x0600,SP
      MOV.W   #0x5a80,&WDTCTL
      BIS.B   #0x000f,&P1DIR
                        CLR.W   R14
        Mainloop:
          MOV.B   R14,&P1OUT
                      ADD.W   #1,R14
          AND.W   #0x000f,R14
        Wait:
          MOV.W   Delay,R15
                       PUSH    R15
        L1:
         SUB.W   #1,0x0000(R1)
                     JNE     (L1)
                     POP.W   R15
                     JMP     (Mainloop)
        Delay:
                      .word   0x0002
