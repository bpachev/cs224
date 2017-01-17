//	pong.h	04/12/2014
//*******************************************************************************
//
//                            MSP430F2274
//                  .-----------------------------.
//            SW1-->|P1.0^                    P2.0|<->LCD_DB0
//            SW2-->|P1.1^                    P2.1|<->LCD_DB1
//            SW3-->|P1.2^                    P2.2|<->LCD_DB2
//            SW4-->|P1.3^                    P2.3|<->LCD_DB3
//       ADXL_INT-->|P1.4                     P2.4|<->LCD_DB4
//        AUX INT-->|P1.5                     P2.5|<->LCD_DB5
//        SERVO_1<--|P1.6 (TA1)               P2.6|<->LCD_DB6
//        SERVO_2<--|P1.7 (TA2)               P2.7|<->LCD_DB7
//                  |                             |
//         LCD_A0<--|P3.0                     P4.0|-->LED_1 (Green)
//        i2c_SDA<->|P3.1 (UCB0SDA)     (TB1) P4.1|-->LED_2 (Orange) / SERVO_3
//        i2c_SCL<--|P3.2 (UCB0SCL)     (TB2) P4.2|-->LED_3 (Yellow) / SERVO_4
//         LCD_RW<--|P3.3                     P4.3|-->LED_4 (Red)
//   TX/LED_5 (G)<--|P3.4 (UCA0TXD)     (TB1) P4.4|-->LCD_BL
//             RX-->|P3.5 (UCA0RXD)     (TB2) P4.5|-->SPEAKER
//           RPOT-->|P3.6 (A6)          (A15) P4.6|-->LED 6 (R)
//           LPOT-->|P3.7 (A7)                P4.7|-->LCD_E
//                  '-----------------------------'
//
//******************************************************************************
#ifndef PONG_H_
#define PONG_H_
// system equates
//
#define myCLOCK	12000000			// clock speed
#define WDT_CLK	32000				// 32 Khz WD clock (@1 Mhz)
#define	WDT_CTL	WDT_MDLY_32			// WDT SMCLK, ~32ms
#define	WDT_CPS	myCLOCK/WDT_CLK		// WD clocks / second count

#define PADDLE_HEIGHT	15
#define PADDLE_MIDDLE	(PADDLE_HEIGHT/2)


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// add service routine events here (bit additive):
#define SWITCH_1		0x0001
#define SWITCH_2		0x0002
#define SWITCH_3		0x0004
#define SWITCH_4		0x0008
#define MOVE_BALL		0x0010
#define ADC_READ		0x0020
#define LCD_UPDATE		0x0040
#define NEW_GAME		0x0080
#define START_GAME		0x0100
#define MISSED_BALL		0x0200
#define END_GAME		0x0400
#define NEW_RALLY		0x0800


#define BEEP_COUNT		myCLOCK/3000
#define RASPBERRY_COUNT		myCLOCK/500
#define BALL_SPEED		32000
#define THRESHHOLD_BALL_SPEED	BALL_SPEED/4
#define POT_THRESHHOLD	3
#define WDT_ADC			WDT_CPS/10		// 100 ms
#define WDT_LCD			WDT_CPS/2		// 500 ms
#define MAX_SCORE		5


#define BEEP TBCCR0 = BEEP_COUNT; TBCCR2 = BEEP_COUNT >> 1; TB0_tone_on = 20;
#define RASPBERRY TBCCR0 = RASPBERRY_COUNT; TBCCR2 = RASPBERRY_COUNT >> 1; TB0_tone_on = 20;
#define DEBOUNCE_CNT	10

enum MODE { IDLE, PLAY, COUNT, EOG, STAT };
enum GAME { PLAYER1, PLAYER2 };

//------------------------------------------------------------------------------
// pong structs
//
typedef struct					// ball struct
{
	int x;						// horizontal position
	int y;						// vertical position
	int old_x;					// previous horizontal position
	int old_y;					// previous vertical position
} BALL;

typedef struct					// paddle struct
{
	int channel;				// ADC id
	int potValue;				// last ADC value
	int x;						// horizontal position
	int y;						// vertical position
	int old_y;					// previous vertical position
} PADDLE;

extern const uint8* count_down[];

//------------------------------------------------------------------------------
// pong prototypes
//
int watchdog_init(void);
int timerA_init(void);
int timerB_init(void);
int port1_init(void);

BALL* new_ball(int x, int y);
void delete_ball(BALL* ball);
int drawBall(BALL* ball);
PADDLE* new_paddle(int channel, int x);
void delete_paddle(PADDLE* paddle);
int drawPaddle(PADDLE* myPaddle);

void MOVE_BALL_event(BALL* ball);
void ADC_READ_event(PADDLE* paddle,int invert);
void NEW_GAME_event(void);
void SWITCH_1_event(void);
void START_GAME_event(void);
void LCD_UPDATE_event(void);
void NEW_RALLY_event(void);
void MISSED_BALL_event(void);
void END_GAME_event(void);

#endif /*PONG_H_*/
