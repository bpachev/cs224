//	pongEvents.c	04/16/2014
//******************************************************************************

#include "msp430x22x4.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "pong.h"

extern volatile int16 dx, dy;
extern volatile uint16 TB0_tone_on;
extern volatile uint16 sys_event;		// pending events

extern volatile uint16 seconds;			// seconds counter
extern volatile uint16 WDT_adc_cnt;
extern volatile uint16 WDT_lcd_cnt;
extern volatile uint16 ball_speed;
extern volatile enum GAME game;
extern volatile int count;

BALL* ball;								// game ball object
PADDLE* rightPaddle;					// right paddle object
PADDLE* leftPaddle;
int player1_score = 0;
int player2_score = 0;
volatile int16 dx, dy;					// ball delta change
volatile enum MODE game_mode = IDLE;			// game mode

extern const uint16 pong_image[];		// 2 paddle image

void updateScore(void);
void drawCenterline(void);
int paddle_center(PADDLE* paddle);
int sign(int num);
int new_ball_speed(int speed);
//BALL myBall;							// **replace with malloc'd struct**
//PADDLE myPaddle;						// **replace with malloc'd struct**

//------------------------------------------------------------------------------
//	ball factory (fix by malloc'ing ball struct)
//
BALL* new_ball(int x, int y)
{
	BALL* p;
	p = (BALL*)malloc(sizeof(BALL));
	if(!p) return 0;
	p->x = x;						// set horizontal position
	p->y = y;						// set vertical position
	p->old_x = p->x;			// set old values
	p->old_y = p->y;
	return p;
} // end init_ball


//------------------------------------------------------------------------------
//	paddle factory (fix by malloc'ing paddle struct)
//
PADDLE* new_paddle(int channel, int x)
{
	PADDLE* p;
	p = (PADDLE*)malloc(sizeof(PADDLE));
	if(!p) return 0;
	p->channel = channel;			// ADC input channel
	p->x = x;						// horizontal paddle position
	p->potValue = -1;				// default all other values
	p->y = -1;
	p->old_y = -1;
	return p;					// return pointer to paddle
} // end init_ball


//------------------------------------------------------------------------------
//	new game event (fix)
//
void NEW_GAME_event(void)
{
	lcd_clear();							// clear LCD
	lcd_volume(345);						// **increase as necessary**
	lcd_backlight(ON);						// turn on LCD
	lcd_wordImage(pong_image, 25, 35, 1);
	lcd_mode(LCD_2X_FONT);					// turn 2x font on
	lcd_cursor(20, 30);						// set display coordinates
	printf("P O N G");
	lcd_mode(~LCD_2X_FONT);					// turn 2x font off
	lcd_cursor(30, 5);
	lcd_printf("PRESS ANY SWITCH");
	game_mode = IDLE;						// idle mode



//************************************************************
//	NEW_RALLY EVENT (move to own event handler
//************************************************************

	// manufacture new ball (NEW_RALLY)
	return;
} // end NEW_GAME_event


//------------------------------------------------------------------------------
//	ADC event - paddle potentiometer and draw paddle (fix)
//
void ADC_READ_event(PADDLE* paddle,int invert)
{
	int pot;
	if (invert) pot = ADC_read(paddle->channel);
	else	pot = 1023-ADC_read(paddle->channel);	// sample potentiometer

	// check for paddle position change
	if ((abs(pot - paddle->potValue) > POT_THRESHHOLD))
	{
		paddle->potValue = pot;			// save old value
		paddle->y = 7+(pot/7);			// update paddle position (fix)
		drawPaddle(paddle);				// draw paddle
	}
	return;
} // end ADC_READ_event


//------------------------------------------------------------------------------
//	TimerA event - move ball (fix)
//
void MOVE_BALL_event(BALL* ball)
{
	if(game_mode!=PLAY) return;
	int right_hit = abs(paddle_center(rightPaddle)-ball->y);
	int left_hit = abs(paddle_center(leftPaddle)-ball->y);
	ball->x += dx;						// increment x coordinate
	ball->y += dy;						// update y coordinate
	if ((ball->y >= HD_Y_MAX-5) || (ball->y < 5))
	{
		dy = -dy;
	}
	drawBall(ball);						// ok, draw ball in new position

	if (ball->x <= 5) 
	{
		player2_score++;
		sys_event |= MISSED_BALL;		
	}
	else if (ball->x >= HD_X_MAX-5)
	{	
		player1_score++;
		sys_event |= MISSED_BALL;
	}
	else if (((ball->x <=7)&&(left_hit<=7))&&(dx<0))
	{
		if (left_hit > 4) dy=2*sign(dy);
		else if ((left_hit>0)&&(left_hit<=4)) dy = sign(dy);
		else if (left_hit==0) dy = 0;
		dx=-dx;
		BEEP;
		ball_speed = new_ball_speed(ball_speed);
	}
	else if (((ball->x >=HD_X_MAX-7)&&(right_hit<=7))&&(dx>0))
	{
		if (right_hit > 4) dy=2*sign(dy);
		else if ((right_hit>0)&&(right_hit<=4)) dy = sign(dy);
		else if (right_hit==0) dy = 0;
		dx=-dx;
		BEEP;
		ball_speed = new_ball_speed(ball_speed);
	}
	return;
} // end MOVE_BALL_event

void START_GAME_event(void)
{
	lcd_clear();
	free(ball);
	ball = 0;
	free(leftPaddle);
	leftPaddle = 0;
	player1_score = 0;
	player2_score = 0;
	if(game==PLAYER2) 
	{
		free(rightPaddle);
		rightPaddle = 0;
		rightPaddle = new_paddle(RIGHT_POT, 157);
		drawPaddle(rightPaddle); 
	}
	leftPaddle = new_paddle(LEFT_POT, 3);
	drawPaddle(leftPaddle);
	WDT_adc_cnt = 1;					// start sampling the paddles
	sys_event |= NEW_RALLY;
}

void LCD_UPDATE_event(void)
{
	switch(game_mode)
	{
		case PLAY:
			drawCenterline();
			updateScore();
			break;
		case COUNT:
			lcd_clear();
			lcd_cursor(80,80);
			if(count>1) lcd_printf("%d",count-1);
			else if(count==1) lcd_printf("GO");
			else if(count==0) sys_event |= START_GAME; 
			count--;
			break;
		default:
			return;
	}
}

void SWITCH_1_event(void)
{
	if(game_mode==IDLE)
	{
		game = PLAYER2;
		count = 4;
		game_mode = COUNT;
		WDT_lcd_cnt = 1;
	}
	else sys_event |= NEW_GAME;
}

void delete_paddle(PADDLE* paddle)
{
	lcd_blank(paddle->x - 1, paddle->y - PADDLE_MIDDLE, 3, 15);
	free(paddle);
}

void delete_ball(BALL* ball)
{
	lcd_blank(ball->x-3,ball->y-3,6,6);
	free(ball);
}

void NEW_RALLY_event(void)
{
	ball = new_ball(80, 80);

	// serve ball
	dx = (rand() % 2) ? -1 : 1;			// delta x
	dy = (rightPaddle->y % 2) ? -1 : 1;			// delta y
	ball_speed = BALL_SPEED;			// interrupt rate
	TACCR0 = ball_speed;				// start timer
	WDT_lcd_cnt = 1;					// start LCD update (ONLY WHEN DEFINED)
	game_mode = PLAY;					// enter play mode
	return;
}

void END_GAME_event(void)
{
	game_mode = EOG;
	lcd_clear();
	delete_paddle(rightPaddle);
	delete_paddle(leftPaddle);
	leftPaddle = 0;
	rightPaddle = 0;
	//lcd_mode(LCD_2X_FONT);					// turn 2x font on
	lcd_cursor(50,150);
	lcd_printf("PLAYER1: %d",player1_score);
	lcd_cursor(50,130);
	lcd_printf("PLAYER2: %d",player2_score);
	lcd_cursor(20,110);
	if(player1_score==MAX_SCORE) lcd_printf("CONGRATULATIONS PLAYER1");
	else if(player2_score==MAX_SCORE) lcd_printf("CONGRATULATIONS PLAYER2");
	//lcd_mode(~LCD_2X_FONT);					// turn 2x font on

	return;
}

void MISSED_BALL_event(void)
{
	RASPBERRY;
	game_mode = IDLE;
	updateScore();
	TACCR0 = 0;
	delete_ball(ball);
	ball = 0;
	if((player1_score<MAX_SCORE)&&(player2_score<MAX_SCORE)) sys_event |= NEW_RALLY;
	else sys_event |= END_GAME;
	return;
}

void drawCenterline(void)
{
	int i;
	for(i = 0; i<20; i++)
	{
		lcd_fillImage(78,i*8,4,4,2);
	}
	return;
}

void updateScore(void)
{
	lcd_mode(LCD_2X_FONT);					// turn 2x font on
	lcd_cursor(65,140);
	lcd_printf("%d",player1_score);
	lcd_cursor(85,140);
	lcd_printf("%d",player2_score);
	lcd_mode(~LCD_2X_FONT);					// turn 2x font on
	return;
}

int paddle_center(PADDLE* paddle)
{
	return 7+(paddle->potValue/7);
}

int sign(int num)
{
	return (num>=0) ? 1 : -1;
}

int new_ball_speed(int speed)
{
	if(speed<=THRESHHOLD_BALL_SPEED) return speed;
	else return speed*0.9;
}
