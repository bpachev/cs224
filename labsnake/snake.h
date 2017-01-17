//------------------------------------------------------------------------------
// snake.h
//------------------------------------------------------------------------------
#ifndef SNAKE_H_
#define SNAKE_H_

//-- system constants ----------------------------------------------------------
#define myCLOCK		1200000				// clock speed
#define CLOCK		_12MHZ

//-- watchdog constants --------------------------------------------------------
#define WDT_CLK		32000				// 32 Khz WD clock (@1 Mhz)
#define	WDT_CTL		WDT_MDLY_32			// WDT SMCLK, ~32ms
#define	WDT_CPS		((12*myCLOCK)/WDT_CLK)		// WD clocks / second count

#define BEEP_COUNT		(myCLOCK/90)
#define RASPBERRY_COUNT		(myCLOCK/30)
#define BEEP TBCCR0 = BEEP_COUNT; TBCCR2 = BEEP_COUNT >> 1; TB0_tone_on = 20;
#define RASPBERRY TBCCR0 = RASPBERRY_COUNT; TBCCR2 = RASPBERRY_COUNT >> 1; TB0_tone_on = 20;
#define TONE(tone_count) TBCCR0 = tone_count; TBCCR2 = tone_count >> 1; TB0_tone_on = 20;


#if WDT_CPS/50
#define DEBOUNCE_CNT	WDT_CPS/50		// 20 ms debounce count
#else
#define DEBOUNCE_CNT	1				// 20 ms debounce count
#endif

#define WDT_LCD		WDT_CPS/4			// 250 ms
#define WDT_MOVE1	(WDT_CPS/4)			// 250 ms
#define WDT_MOVE2	(WDT_CPS/8)			// 125 ms
#define WDT_MOVE3	(WDT_CPS/16)			// 62 ms
#define WDT_MOVE4	(WDT_CPS/32)			// 31 ms

//-- sys_events ----------------------------------------------------------------
#define SWITCH_1	0x0001
#define SWITCH_2	0x0002
#define SWITCH_3	0x0004
#define SWITCH_4	0x0008

#define START_LEVEL	0x0010
#define NEXT_LEVEL	0x0020
#define END_GAME	0x0040
#define NEW_GAME	0x0080

#define MOVE_SNAKE	0x0100
#define LCD_UPDATE	0x0200

//-- service routine events ----------------------------------------------------
void SWITCH_1_event(void);
void SWITCH_2_event(void);
void SWITCH_3_event(void);
void SWITCH_4_event(void);

void START_LEVEL_event(void);
void NEXT_LEVEL_event(void);
void END_GAME_event(void);
void NEW_GAME_event(void);

void MOVE_SNAKE_event(void);
void LCD_UPDATE_event(void);
int timerB_init(void);

//-- snake game equates --------------------------------------------------------
#define START_SCORE			10
#define X_MAX	24						// columns
#define Y_MAX	23						// rows

#define MAX_SNAKE			64			// max snake length 
#define MAX_FOOD			10			// max # of foods
#define MAX_ROCKS			4
#define MIN_ROCKS			2

#define TIME_1_LIMIT		30
#define LEVEL_1_FOOD		10			// 10
#define LEVEL_1_POINTS		1

#define TIME_2_LIMIT		30
#define LEVEL_2_FOOD		MAX_FOOD
#define LEVEL_2_POINTS		2


#define TIME_3_LIMIT		45
#define LEVEL_3_FOOD		MAX_FOOD
#define LEVEL_3_POINTS		3


#define TIME_4_LIMIT		60
#define LEVEL_4_FOOD		MAX_FOOD
#define LEVEL_4_POINTS		4

#define TONE_C		myCLOCK/1308/12*10
#define TONE_Cs		myCLOCK/1386/12*10
#define TONE_D		myCLOCK/1468/12*10
#define TONE_Eb		myCLOCK/1556/12*10
#define TONE_E		myCLOCK/1648/12*10
#define TONE_F		myCLOCK/1746/12*10
#define TONE_Fs		myCLOCK/1850/12*10
#define TONE_G		myCLOCK/1950/12*10
#define TONE_Ab		myCLOCK/2077/12*10
#define TONE_A		myCLOCK/2200/12*10
#define TONE_Bb		myCLOCK/2331/12*10
#define TONE_B		myCLOCK/2469/12*10

enum {RIGHT, UP, LEFT, DOWN};			// movement constants
enum modes {IDLE, PLAY, NEXT};			// player modes

typedef struct							// POINT struct
{
	uint8 x;
	uint8 y;
} POINT;

typedef union							// snake segment object
{
	uint16 xy;
	POINT point;
} SNAKE;

typedef union
{
	uint16 xy;
	POINT point;
} ROCK;

typedef struct food_struct
{
   union
   {
      uint16 xy;         // 16-bit food coordinate ((y << 8) + x)
      POINT point;       // 2 8-bit food coordinates (x, y)
   } food;
   uint8 size;           // pixel size of food
   uint8 points;         // value of food (not used)
   void (*draw)(struct food_struct* food, uint8 pen);
} FOOD;

#define COL(x)	(x*6+7+3)				// grid x value to LCD column
#define ROW(y)	(y*6+10+3)				// grid y value to LCD row

#endif /* SNAKE_H_ */
