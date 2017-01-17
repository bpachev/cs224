//	pong.c	04/08/2014
//******************************************************************************
//  Pong
//
//  Description:
//
//	Write a C program to play a 2-player game of Pong, a two-dimensional sports
//	game which simulates table tennis. Use the potentiometers to control the
//	paddles moving vertically on each side of the screen. The paddles and walls
//	reflect a ball back and forth at the correct speed and reflection angle.
//	A point is scored when one side fails to return the ball to the other side.
//	Show the score in real time on the LCD display.
//
//	Divide the paddle into segments to change the ball's angle of return.
//	For example, the center segments return the ball a 90° angle in relation to
//	the paddle, while the outer segments return the ball at smaller angles.
//	Make the ball accelerate the more it is returned back and forth between
//	paddles; missing the ball resets the speed.
//
//  Author:		Benjamin Pachev
// This completed assignment is my own work.



#include "msp430x22x4.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "pong.h"

//------------------------------------------------------------------------------
// pong global variables
volatile uint16 sys_event;				// pending events
extern BALL* ball;						// game ball object
extern PADDLE* rightPaddle;				// right paddle object
extern PADDLE* leftPaddle;
volatile enum GAME game = PLAYER2;
volatile int count = 4;

//------------------------------------------------------------------------------
//
void main(void)
{
	ERROR2(RBX430_init(_12MHZ));		// init board to 12 MHz
	ERROR2(lcd_init());					// init LCD
	ERROR2(port1_init());				// init switches
	ERROR2(ADC_init());					// init ADC
	ERROR2(watchdog_init());			// init watchdog
	ERROR2(timerA_init());				// init TimerA
	ERROR2(timerB_init());				// init TimerB
	__bis_SR_register(GIE);				// enable interrupts
	sys_event = NEW_GAME;				// start w/new game

	//-----------------------------------------------------------
	//	play forever
	while (1)
	{
		//-----------------------------------------------------------
		//	event service routine loop
		//-----------------------------------------------------------
		while (1)
		{
			// disable interrupts while checking sys_event
			__bic_SR_register(GIE);	


			// if event pending, enable interrupts
			if (sys_event) 	__bis_SR_register(GIE);


			// else enable interrupts and goto sleep
			else __bis_SR_register(LPM0_bits | GIE);

			//-------------------------------------------------------
			//	I'm AWAKE!!!  What needs service?
			if (sys_event & MOVE_BALL)				// timer A event
			{
				sys_event &= ~MOVE_BALL;			// clear TimerA event
				MOVE_BALL_event(ball);				// update ball position
			}
			else if (sys_event & ADC_READ)			// read ADC event
			{
				sys_event &= ~ADC_READ;				// clear ADC event
				if(game==PLAYER2) ADC_READ_event(rightPaddle,0);		// process ADC read
				ADC_READ_event(leftPaddle,1);
			}
			else if (sys_event & LCD_UPDATE)
			{
				sys_event &= ~LCD_UPDATE;
				LCD_UPDATE_event();
			}
			else if (sys_event & MISSED_BALL)
			{
				sys_event &= ~MISSED_BALL;
				MISSED_BALL_event();
			}
			else if (sys_event & NEW_RALLY)
			{
				sys_event &= ~NEW_RALLY;
				NEW_RALLY_event();
			}
			else if (sys_event & NEW_GAME)			// new game event
			{
				sys_event &= ~NEW_GAME;				// clear new game event
				NEW_GAME_event();					// process new game
			}
			else if (sys_event & START_GAME)
			{
				sys_event &= ~START_GAME;
				START_GAME_event();
			}
			else if (sys_event & END_GAME)
			{
				sys_event &= ~END_GAME;
				END_GAME_event();
			}
			else if (sys_event & SWITCH_1)
			{
				sys_event &= ~SWITCH_1;
				SWITCH_1_event();
			}
			else									// ????
			{
				ERROR2(SYS_ERR_EVENT);				// unrecognized event
			}
		}
	}
} // end main
