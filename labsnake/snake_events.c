//******************************************************************************
//	snake_events.c
//
//  Author:			Benjamin Pachev
// This completed assignment is my own work.
//
//******************************************************************************
//
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "snake.h"

extern volatile uint16 sys_event;			// pending events
extern volatile uint16 move_cnt;			// snake speed
extern volatile uint16 TB0_tone_on;

extern volatile enum modes game_mode;		// 0=idle, 1=play, 2=next
extern volatile uint16 score;				// current score
extern volatile uint16 level;				// current level (1-4)
extern volatile uint16 direction;			// current move direction
extern volatile uint16 WDT_move_cnt;

extern volatile uint8 head;					// head index into snake array
extern volatile uint8 tail;					// tail index into snake array
extern SNAKE snake[MAX_SNAKE];				// snake segments
extern ROCK  rocks[MAX_ROCKS];
extern FOOD* foods[MAX_FOOD];

extern const uint16 snake_text_image[];
extern const uint16 snake1_image[];

volatile uint8 time = 0;

static uint8 move_right(SNAKE* head);		// move snake head right
static uint8 move_up(SNAKE* head);			// move snake head up
static uint8 move_left(SNAKE* head);		// move snake head left
static uint8 move_down(SNAKE* head);		// move snake head down
static void new_snake(uint16 length, uint8 dir);
static void delete_tail(void);
static void add_head(void);
void draw_button_labels(void);
void createFood(FOOD** food, uint16 xy,int points);
void init_foods(int num_foods,int points);
void init_rocks(void);
void draw_triangle_food(FOOD* food,uint8 pen);
void draw_square_food(FOOD* food,uint8 pen);
void draw_circle_food(FOOD* food,uint8 pen);
void die(char* msg);
void delete_foods(void);
uint16 get_free_xy(void);




//-- switch #1 event -----------------------------------------------------------
//
void SWITCH_1_event(void)
{
	switch (game_mode)
	{
		case IDLE:
			sys_event |= NEW_GAME;
			break;

		case PLAY:
			if (direction != LEFT)
			{
				if (snake[head].point.x < X_MAX)
				{
					direction = RIGHT;
					sys_event |= MOVE_SNAKE;
				}
			}
			break;

		case NEXT:
			sys_event |= START_LEVEL;
			break;
	}
	return;
} // end SWITCH_1_event


//-- switch #2 event -----------------------------------------------------------
//
void SWITCH_2_event(void)
{
	if((game_mode==PLAY)&&(direction!=RIGHT))
	{
		if (snake[head].point.x > 0)
		{
			direction = LEFT;
			sys_event |= MOVE_SNAKE;
		}
	}
} // end SWITCH_2_event


//-- switch #3 event -----------------------------------------------------------
//
void SWITCH_3_event(void)
{
	if((game_mode==PLAY)&&(direction!=UP))
	{
		if (snake[head].point.y > 0)
		{
			direction = DOWN;
			sys_event |= MOVE_SNAKE;
		}
	}
} // end SWITCH_3_event


//-- switch #4 event -----------------------------------------------------------
//
void SWITCH_4_event(void)
{
	if((game_mode==PLAY)&&(direction!=DOWN))
	{
		if (snake[head].point.x < Y_MAX)
		{
			direction = UP;
			sys_event |= MOVE_SNAKE;
		}
	}
} // end SWITCH_4_event


//-- next level event -----------------------------------------------------------
//
void NEXT_LEVEL_event(void)
{
	game_mode = IDLE;
	if(level==4) 
	{
		sys_event |= END_GAME;
		return;
	}
	level++;
	delete_foods();
	TONE(TONE_D*10);
	TONE(TONE_E*10);
	TONE(TONE_F*10);
	sys_event |= START_LEVEL;
	return;
} // end NEXT_LEVEL_event


//-- update LCD event -----------------------------------------------------------
//
void LCD_UPDATE_event(void)
{
//	if(game_mode==PLAY)
//	{
		lcd_cursor(5,150);
		lcd_printf("Score: %d",score);
		lcd_cursor(60,150);
		lcd_printf("Time:%d ",time);
		lcd_cursor(110,150);
		lcd_printf("Level: %d",level);
		if(time==0) sys_event |= END_GAME;
//	}
	else return;
} // end LCD_UPDATE_event


//-- end game event -------------------------------------------------------------
//
void END_GAME_event(void)
{
	game_mode = IDLE;
	delete_foods();
	lcd_cursor(30,30);
	lcd_printf("SCORE: %d",score);
	if(score<20)
	{
		lcd_cursor(30,50);
		lcd_printf("THREADSNAKE (epic fail)");
	}
	else if((score>=20)&&(score<30))
	{
		lcd_cursor(30,50);
		lcd_printf("GARTER SNAKE (poor)");
	}
	else if((score>=30)&&(score<45))
	{
		lcd_cursor(30,50);
		lcd_printf("ADDER (average)");
	}
	else if((score>=45)&&(score<50))
	{
		lcd_cursor(30,50);
		lcd_printf("VIPER (fair)");
	}
	else if((score>=60)&&(score<80))
	{
		lcd_cursor(30,50);
		lcd_printf("COBRA (good)");
	}
	else if((score>=80)&&(score<90))
	{
		lcd_cursor(30,50);
		lcd_printf("ANACONDA (very good)");
	}
	else if((score>=90)&&(score<100))
	{
		lcd_cursor(30,50);
		lcd_printf("PYTHON (excellent)");
	}
	else if(score==100)
	{
		lcd_cursor(30,50);
		lcd_printf("KING SNAKE!!! (perfect)");
	}
	score = 0;
	level = 1;
	
} // end END_GAME_event


//-- move snake event ----------------------------------------------------------
//
void MOVE_SNAKE_event(void)
{
	int i = 0;
	int food_eaten = 0;
	int num_foods = (level<3) ? MAX_FOOD : 1;
	if (level > 0)
	{
		add_head();						// add head
		lcd_point(COL(snake[head].point.x), ROW(snake[head].point.y), PENTX);
		for(i=0;i<num_foods;i++)
		{
			if(foods[i]->food.xy==snake[head].xy)
			{
				lcd_backlight(OFF);
				food_eaten=1;
				foods[i]->food.xy = get_free_xy();
				if(level!=2) (foods[i]->draw)(foods[i], SINGLE);         // draw food
				else foods[i]->food.xy = ((Y_MAX << 8) + X_MAX);
				score+=foods[i]->points;
				sys_event |= LCD_UPDATE;
				BEEP;
				lcd_backlight(ON);
				if(score==(5*level*(level+1))) sys_event |= NEXT_LEVEL;
			}
		}
		if(!food_eaten) delete_tail();					// delete tail
		for(i = tail; i!=((head) & (~MAX_SNAKE)); i = (i+1)&(~MAX_SNAKE))
		{
			if (snake[head].xy==snake[i].xy) 
			{
				RASPBERRY;
				sys_event |= END_GAME;
				return;
			}
		}
		for(i=0;i<MAX_ROCKS;i++)
		{

			if(snake[head].xy==rocks[i].xy)
			{
				RASPBERRY;
				sys_event |= END_GAME;
				return;				
			}
		}	

	}
	return;
} // end MOVE_SNAKE_event


//-- start level event -----------------------------------------------------------
//
void START_LEVEL_event(void)
{
	lcd_clear();
	draw_button_labels();
	new_snake(level*10, RIGHT);
	switch(level)
	{
		case 1:
			time = TIME_1_LIMIT;
			move_cnt = WDT_MOVE1;
			WDT_move_cnt = move_cnt;
			init_foods(MAX_FOOD,LEVEL_1_POINTS);
			break;
		case 2:
			time = TIME_2_LIMIT;
			move_cnt = WDT_MOVE2;
			WDT_move_cnt = move_cnt;
			init_foods(MAX_FOOD,LEVEL_2_POINTS);
			break;
		case 3:
			time = TIME_3_LIMIT;
			move_cnt = WDT_MOVE3;
			WDT_move_cnt = move_cnt;
			init_foods(1,LEVEL_3_POINTS);
			break;
		case 4:
			time = TIME_4_LIMIT;
			move_cnt = WDT_MOVE4;
			WDT_move_cnt = move_cnt;
			init_foods(1,LEVEL_4_POINTS);
			break;
		default:
			break;
	}
	init_rocks();
	game_mode = PLAY;					// start level
//				die("foods init");
	return;
} // end START_LEVEL_event


//-- new game event ------------------------------------------------------------
//
void NEW_GAME_event(void)
{
	lcd_clear();						// clear lcd
	lcd_backlight(1);					// turn on backlight
	lcd_wordImage(snake1_image, (159-60)/2, 60, 1);
	lcd_wordImage(snake_text_image, (159-111)/2, 20, 1);
	level = 1;
	score = 0;
	game_mode = NEXT;
	return;
} // end NEW_GAME_event


//-- new snake -----------------------------------------------------------------
//
void new_snake(uint16 length, uint8 dir)
{
	int i;
	head = 0;
	tail = 0;
	snake[head].point.x = 0;
	snake[head].point.y = 0;
	direction = dir;

// 	// build snake
	for (i = length - 1; i > 0; --i)
	{
		add_head();
		lcd_point(COL(snake[head].point.x), ROW(snake[head].point.y), PENTX);

	}
	return;
} // end new_snake


//-- delete_tail  --------------------------------------------------------------
//
void delete_tail(void)
{
	lcd_point(COL(snake[tail].point.x), ROW(snake[tail].point.y), PENTX_OFF);
	tail = (tail + 1) & (~MAX_SNAKE);
} // end delete_tail


//-- add_head  -----------------------------------------------------------------
//
void add_head(void)
{
	static uint8 (*mFuncs[])(SNAKE*) =	// move head function pointers
	             { move_right, move_up, move_left, move_down };
	uint8 new_head = (head + 1) & (~MAX_SNAKE);
	snake[new_head] = snake[head];		// set new head to previous head
	head = new_head;
	// iterate until valid move
	while ((*mFuncs[direction])(&snake[head]));
} // end add_head


//-- move snake head right -----------------------------------------------------
//
uint8 move_right(SNAKE* head)
{
	if ((head->point.x + 1) < X_MAX)		// room to move right?
	{
		++(head->point.x);					// y, move right
		return FALSE;
	}
	if ((level!=2)&&(game_mode==PLAY))							// n, right fence
	{
		if (level > 2) 
		{
			sys_event = END_GAME;
			RASPBERRY;
		}
		head->point.x = 0;					// wrap around
		return FALSE;
	}
	if (head->point.y) direction = DOWN;	// move up/down
	else direction = UP;
	return TRUE;
} // end move_right


//-- move snake head up --------------------------------------------------------
//
uint8 move_up(SNAKE* head)
{
	if ((head->point.y + 1) < Y_MAX)
	{
		++(head->point.y);					// move up
		return FALSE;
	}
	if (level!=2)							// top fence
	{
		if (level > 2) 
		{
			sys_event = END_GAME;
			RASPBERRY;
		}
		head->point.y = 0;					// wrap around
		return FALSE;
	}
	if (head->point.x) direction = LEFT;	// move left/right
	else direction = RIGHT;
	return TRUE;
} // end move_up


//-- move snake head left ------------------------------------------------------
//
uint8 move_left(SNAKE* head)
{
	if (head->point.x)
	{
		--(head->point.x);					// move left
		return FALSE;
	}
	if (level!=2)							// left fence
	{
		if (level> 2) 
		{
			sys_event = END_GAME;
			RASPBERRY;
		}
		head->point.x = X_MAX - 1;			// wrap around
		return FALSE;
	}
	if (head->point.y) direction = DOWN;	// move down/up
	else direction = UP;
	return TRUE;
} // end move_left


//-- move snake head down ------------------------------------------------------
//
uint8 move_down(SNAKE* head)
{
	if (head->point.y)
	{
		--(head->point.y);					// move down
		return FALSE;
	}
	if (level!=2)							// bottom fence
	{
		if (level > 2) 
		{
			sys_event = END_GAME;
			RASPBERRY;
		}
		head->point.y = Y_MAX - 1;			// wrap around
		return FALSE;
	}
	if (head->point.x) direction = LEFT;	// move left/right
	else direction = RIGHT;
	return TRUE;
} // end move_down

void draw_button_labels(void)
{
	lcd_cursor(5,0);
	lcd_printf("UP");
	lcd_cursor(45,0);
	lcd_printf("DOWN");
	lcd_cursor(95,0);
	lcd_printf("LEFT");
	lcd_cursor(130,0);
	lcd_printf("RIGHT");	
}

void createFood(FOOD** food, uint16 xy,int points)
{
   // malloc a food struct

   if (!(*food = (FOOD*)malloc(sizeof(FOOD)))) die("malloc error");

   (*food)->food.xy = xy;              // add coordinates to food
   (*food)->points = points;

   switch (((uint8)rand()) % 3)                    // randomly choose a type
   {
	case 0:                              // square food
		(*food)->size = 3;                // 2 pixels in size
		(*food)->draw = draw_square_food; // add function pointer to
		break;                            //   draw square
	case 1:
	 	(*food)->size = 2;
		(*food)->draw = draw_triangle_food;
		break;
	case 2:
		(*food)->size = 2;
		(*food)->draw = draw_circle_food;
		break;
	default:
		(*food)->size = 4;                // 2 pixels in size
		(*food)->draw = draw_square_food; // add function pointer to
		break;
		
   }

   ((*food)->draw)(*food, SINGLE);         // draw food
   //                  if(points>1) die("food 2");

   return;
}

void draw_square_food(FOOD* food, uint8 pen)
{
   lcd_rectangle(COL(food->food.point.x),
              ROW(food->food.point.y),
              food->size,food->size, pen);
}

void draw_triangle_food(FOOD* food, uint8 pen)
{
   lcd_triangle(COL(food->food.point.x),
              ROW(food->food.point.y),
              food->size, pen);
}

void draw_circle_food(FOOD* food, uint8 pen)
{
   lcd_circle(COL(food->food.point.x),
              ROW(food->food.point.y),
              food->size, pen);
}

void init_foods(int num_foods,int points)
{
//	uint16 food_locs[num_foods];
	uint16 food_xy = 0;
	int i;

	for(i=0;i < num_foods;i++)
	{
		food_xy = get_free_xy();
//		food_locs[i] = ((food_y << 8)+food_x);
		createFood(foods+i,food_xy,points);
	}
	
}

void die(char *msg)
{
	lcd_cursor(30,30);
	lcd_printf("msg: %s",msg);
	ERROR2(10);
}

void delete_foods(void)
{
	int num_foods = (level>2) ? 1 : MAX_FOOD;
	int i = 0;
	for(i = 0; i<num_foods; i++)
	{
		free(foods[i]);
	}
	return;
}

uint16 get_free_xy(void)
{
	uint8 x = ((uint8)rand())%X_MAX;
	uint8 y = ((uint8)rand())%Y_MAX;
	//x=1;
	//y=1;
	uint16 xy = ((y << 8) + x);
	int i = 0;
	int num_foods = (level<3) ? MAX_FOOD : 1;
	for(i = 0; i<num_foods; i++)
	{
		if (xy==foods[i]->food.xy) xy = get_free_xy();
	}
	for(i = tail; i!=((head+1) & (~MAX_SNAKE)); i = (i+1)&(~MAX_SNAKE))
	{
		if (xy==snake[i].xy) xy = get_free_xy();
	}
	for(i = 0; i<MAX_ROCKS; i++)
	{
		if(xy==rocks[i].xy) xy = get_free_xy();
	}
	return xy;
}

void init_rocks(void)
{
	int num_rocks = MIN_ROCKS + (((uint8)rand()) % (MAX_ROCKS-MIN_ROCKS+1));
	int i = 0;
	for(i=0;i<MAX_ROCKS;i++)
	{
		if(i<num_rocks) 
		{
			rocks[i].xy = get_free_xy();
			lcd_point(COL(rocks[i].point.x), ROW(rocks[i].point.y), PENTX);

		}
		else rocks[i].xy = ((Y_MAX << 8) + X_MAX);	
	}
}