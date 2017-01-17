//	Lab01.c	2014/06/16
#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lab01.h"

int main(void)
{
    signed char biggest_char, next_char;
	lab01_init();

	TERMINAL("\n\r** INTEGERS *************");
  	TERMINAL1("Signed int: 21000 + 200 = %d", 21000 + 200);

    next_char = 1;						// start w/1
    biggest_char = 1;
    while((next_char <<= 1) > 0)
    {
    	biggest_char = (biggest_char << 1) + 1;;
    }
	TERMINAL2("Largest signed char = %d (0x%2x)", biggest_char);
	TERMINAL3("Largest Q4.4 = %f (0x%2x)", (((long)biggest_char)/16.0), biggest_char);
    return 0;
}
