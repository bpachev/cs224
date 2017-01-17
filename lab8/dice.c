//******************************************************************************
//  Lab 8a - Dice
//
//  Description:
//
//	"Write a dice roller C program that waits for a switch to be pressed and then
//	 displays two dice which randomly change values. The dice roll begins rapidly
//	 and progressively slows down until it stops (after approximately 3-5
//	 seconds). A congratulations ditty is played when doubles are rolled. If
//	 doubles are rolled twice in a row, output a raspberry tone. Write an
//	 assembly language function which returns a random number from 1 to 6 and
//	 call this function from your C program." 
//
//  Author:		Benjamin Pachev
//This completed assignment is my own work.
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "dice.h"



#define DEBOUNCE_CNT	20

// external/internal prototypes ------------------------------------------------
extern int setrandSeed(int seed);
extern uint16 random(int low, int high);	// get random # between low and high
extern void doTone(uint16 tone, uint16 time);
extern void doDitty(struct tone *ditty, int ditty_length);
extern void display_frq_histogram(uint8 *frq_arr,int frq_length);
uint16 myMod(int n, int m);					// return (n % m)
void handle_switch(void);
extern void init_die_screen(void);
extern volatile uint16 seconds;				// second counter
extern volatile SCREEN_STATE screen;
volatile uint8 Die1_last = 0;
volatile uint8 Die2_last = 0;
volatile int WDT_debounce_cnt = 0;
volatile int max_roll_frq = 0;
volatile int switches;
struct tone charge_ditty[] = {{TONE_D,WDT_1SEC_CNT/5},{TONE_E,WDT_1SEC_CNT/5},{TONE_F,WDT_1SEC_CNT/5}};
uint8 roll_frqs[] = {0,0,0,0,0,0,0,0,0,0,0};
int doubles_last = 0;
volatile int inactivity_cnt = MAX_IDLE_CNT;


//------------------------------------------------------------------------------
// main ------------------------------------------------------------------------
void main(void)
{
	RBX430_init(1);						// init board
	ERROR2(lcd_init());						// init LCD
	ERROR2(WDT_init());						// init watchdog as timer
	ERROR2(timerB_init());					// init timerB for tones
	__bis_SR_register(GIE);					// enable interrupts

	init_die_screen();
	setrandSeed(ADC_read(MSP430_TEMPERATURE) + ADC_read(RED_LED));

	while (1)								// repeat forever
	{
		P1IES |= 0x0f;                   // interrupt on h to l transition
		P1IE |= 0x0f;                    // enable P1.0-3 interrupts
		if(switches)
		{
			if((switches & 0x08)!=0)
			{
				display_frq_histogram(roll_frqs,sizeof(roll_frqs)/sizeof(uint8));
			}
			else
			{
				if(screen==DIE) handle_switch();
			}
		}
//		doTone(TONE_F,700);
//		doTone(BEEP, BEEP_CNT);				// output BEEP
//	lcd_cursor(10, 15);					// position message
//	lcd_printf("\b\tRoll again! %d",i++);

//		WDT_delay(1000);						// delay
	}
} // end main()

uint16 myMod (int n, int m)
{
	return n%m;
}


void handle_switch (void)
{
	if(screen==FRQ) init_die_screen();
	lcd_blank(10,15,150,15);
	int roll_delay = 0;
	uint8 Die1 = 1;
	uint8 Die2 = 1;
	while (roll_delay<100)
	{
		roll_delay+=10;
		Die1 = random(1,6);
		Die2 = random(1,6);
		drawDie(Die1,15,51);
		drawDie(Die2,96,51);
		int roll_tone_length = (roll_delay<40) ? roll_delay : 40;
		doTone(TONE_C,roll_tone_length);
		WDT_delay(roll_delay);
		if ((P1IN & 0x0f)!=(0x0f))
		{
			roll_delay = 0;
			inactivity_cnt = MAX_IDLE_CNT;
		}	
	}
	lcd_cursor(10, 15);					// position message
	lcd_printf("Roll again!");
	doTone(TONE_F,100);
	WDT_delay(150);
	if(Die1==Die2) {
		doDitty(charge_ditty,sizeof(charge_ditty)/sizeof(struct tone));
		if(doubles_last==1) {
			doTone(TONE_Eb,WDT_1SEC_CNT);
			WDT_delay(WDT_1SEC_CNT);
		}
		doubles_last=1;
	}
	else doubles_last = 0;
	Die1_last = Die1;
	Die2_last = Die2;
	roll_frqs[Die1+Die2-2]++;
	int current_roll_frq = roll_frqs[Die1+Die2-2];
	if (max_roll_frq<current_roll_frq) max_roll_frq=current_roll_frq;
	inactivity_cnt = MAX_IDLE_CNT;
	switches=0;
	_BIS_SR(LPM0_bits+GIE);
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void)
{
   P1IFG &= ~0x0f;                  // P1.0-3 IFG cleared
   WDT_debounce_cnt = DEBOUNCE_CNT; // enable debounce
   LPM0_EXIT;
} // end Port_1_ISR


// Watchdog Timer ISR -----------------------------------------------

