//	life.c	03/25/2014
//******************************************************************************
//  The Game of Life
//
//  Lab Description:
//
//  The universe of the Game of Life is an infinite two-dimensional orthogonal
//  grid of square cells, each of which is in one of two states, alive or dead.
//  With each new generation, every cell interacts with its eight neighbors,
//  which are the cells horizontally, vertically, or diagonally adjacent
//  according to the following rules:
//
//  1. A live cell stays alive (survives) if it has 2 or 3 live neighbors,
//     otherwise it dies.
//  2. A dead cell comes to life (birth) if it has exactly 3 live neighbors,
//     otherwise it stays dead.
//
//  An initial set of patterns constitutes the seed of the simulation. Each
//  successive generation is created by applying the above rules simultaneously
//  to every cell in the current generation (ie. births and deaths occur
//  simultaneously.)  See http://en.wikipedia.org/wiki/Conway's_Game_of_Life
//
//  Author:    Benjamin Pachev

//
//  This completed assignment is my own work
//******************************************************************************
//  Lab hints:
//
//  The life grid (uint8 life[80][10]) is an 80 row x 80 column bit array.  A 0
//  bit is a dead cell while a 1 bit is a live cell.  The outer cells are always
//  dead.  A boolean cell value (0 or non-zero) is referenced by:
//
//         life[row][col >> 3] & (0x80 >> (col & 0x07))
//
//  Each life cell maps to a 2x2 lcd pixel.
//
//                     00       01             08       09
//  life[79][0-9]   00000000 00000000  ...  00000000 00000000 --> life_pr[0-9]
//  life[78][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_cr[0-9]
//  life[77][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_nr[0-9]
//  life[76][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0         |
//     ...                                                            |
//  life[75-4][0-9]   ...      ...            ...      ...            v
//     ...
//  life[03][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[02][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[01][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[00][0-9]   00000000 00000000  ...  00000000 00000000
//
//  The next generation can be made directly in the life array if the previous
//  cell values are held in the life_pr (previous row), life_cr (current row),
//  and life_nr (next row) arrays and used to count cell neighbors.
//
//  Begin each new row by moving life_cr values to life_pr, life_nr values to
//  life_cr, and loading life_nr with the row-1 life values.  Then for each
//  column, use these saved values in life_pr, life_cr, and life_nr to
//  calculate the number of cell neighbors of the current row and make changes
//  directly in the life array.
//
//  life_pr[0-9] = life_cr[0-9]
//  life_cr[0-9] = life_nr[0-9]
//  life_nr[0-9] = life[row-1][0-9]
//
//******************************************************************************
//******************************************************************************
// includes --------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "msp430.h"
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "life.h"

// global variables ------------------------------------------------------------
extern volatile uint16 WDT_Sec_Cnt;		// WDT second counter
extern volatile uint16 seconds;			// # of seconds
extern volatile uint16 switches;		// debounced switch values

uint8 life[NUM_ROWS][NUM_COLS/8];		// 80 x 80 life grid
uint8 life_buff[3][NUM_COLS/8];
uint8* life_pr, *life_cr,*life_nr,*dummy_arr;


uint16 pen = 1;							// ** delete **

int inline get_neighbors(uint16 row, uint16 col,int col_div8,int col_rem);
int inline get_neighbors_fast(int cr_byte, int nr_byte, int pr_byte, uint16 row, uint16 col,int col_rem,int col_div8);
int inline get_neighbors_prev(int cr_byte, int nr_byte, int pr_byte, uint16 row, uint16 col,int col_rem,int col_div8);
int inline get_neighbors_next(int cr_byte, int nr_byte, int pr_byte, uint16 row, uint16 col,int col_rem,int col_div8);
void inline cell_birth(uint16 row, uint16 col);
void inline cell_death(uint16 row, uint16 col);
//------------------------------------------------------------------------------
//	draw RLE pattern -----------------------------------------------------------
void draw_rle_pattern(int row, int col, const uint8* object)
{
	int run_count = 0;
	int start_col=col;
	int i;
	int spotted_height = 0;
	const uint8* ptr = object;
	while (*ptr && (*ptr++ != '\n'))
	{
		if (*ptr == 'y') spotted_height = 1;
		if (spotted_height) 
		{
			while(isdigit(*ptr)) run_count = 10*run_count + (*ptr++ - '0');
			if (run_count)
			{
				row += run_count - 1;
				spotted_height = 0;
				run_count = 0;
			}

		}
	}
	while (*ptr != '!')
	{
		if (row==0) return;
		if (isdigit(*ptr)) run_count = run_count*10 + (*ptr-'0');
		else if (*ptr=='b')
		{
			if(!run_count) run_count = 1;
			for(i=1;i<=run_count;i++)
			{
				cell_death(row,col++);
			}
			run_count=0;
		}
		else if (*ptr=='o')
		{
			if(!run_count) run_count = 1;
			for(i=1;i<=run_count;i++)
			{
				cell_birth(row,col++);
			}
			run_count=0;
		}
		else if (*ptr=='$')
		{
			if(!run_count) run_count = 1;
			row-=run_count;
			run_count=0;
			col=start_col;
		}
		ptr++;
	}
	return;
} // end draw_rle_pattern


//------------------------------------------------------------------------------
// main ------------------------------------------------------------------------
void main(void)
{
	RBX430_init(_16MHZ);					// init board
	ERROR2(lcd_init());					// init LCD
	//lcd_volume(376);					// increase LCD brightness
	watchdog_init();					// init watchdog
	port1_init();						// init P1.0-3 switches
	__bis_SR_register(GIE);				// enable interrupts

	while (1)							// new pattern seed
	{
		uint16 generation;				// generation counter
		uint16 row, col;
		register int neighbors;
		int cr_byte = 0;
		int nr_byte = 0;
		int pr_byte = 0;
		register uint8 cell;

		// setup beginning life generation
		if(switches) init_life(switches);
		else init_life(BIRD);				// load a new life seed into LCD


		WDT_Sec_Cnt = 16*WDT_1SEC_CNT;		// reset WD 1 second counter
		seconds = 0;					// clear second counter
		switches = 0;					// clear switch variable
		generation = 0;					// start generation counter


		while (1)						// next generation
		{
			// for each life row (78 down to 1)
			memset(&life_buff[0][0],0,NUM_COLS/8);
			life_cr = (uint8 *)memcpy(&life_buff[1][0],&life[NUM_ROWS-2][0],NUM_COLS/8);
			life_nr = (uint8 *)memcpy(&life_buff[2][0],&life[NUM_ROWS-3][0],NUM_COLS/8);
			life_pr = &life_buff[0][0];
			uint8 register life_byte = 0;
			for (row = NUM_ROWS-2; row; row -= 1)
			{
				
				// for each life column (78 down to 1)
				for (col = NUM_COLS-2; col; col -= 1)
				{
					register int col_div8 = col >> 3;
					int col_rem = 0x80 >> (col & 0x07);
					if((col%8==7)||(col==NUM_COLS-2))
					{
						cr_byte = life_cr[col_div8];
						nr_byte = life_nr[col_div8];
						pr_byte = life_pr[col_div8];
					}
					cell = (life_cr[col_div8] & col_rem);
					//tmp = (life_pr[(col) >> 3] & (0x80 >> ((col) & 0x07)));
					
					switch(col%8)
					{
						case 0:
							neighbors = get_neighbors_prev(cr_byte,nr_byte,pr_byte,row,col,col_rem,col_div8);
							break;
						case 7: 
							neighbors = get_neighbors_next(cr_byte,nr_byte,pr_byte,row,col,col_rem,col_div8);
							break;
						default:
							neighbors = get_neighbors_fast(cr_byte,nr_byte,pr_byte,row,col,col_rem,col_div8);
							break;
					}
					if(neighbors<2&&cell)
					{
						clear_cell(row,col);
					}
					else if(neighbors==3&&!cell)
					{
						fill_cell(row,col);
						life_byte |= (0x80 >> ((col) & 0x07));
					}
					else if(neighbors>3&&cell)
					{
						clear_cell(row,col);
					}
					else life_byte += cell;
					if((col%8==0)||(col==1))
					{
						life[row][col_div8] = life_byte;
						life_byte = 0;
					}
				}
				dummy_arr = life_pr;
				life_pr=life_cr;
				life_cr=life_nr;
				life_nr=dummy_arr;
				memcpy(life_nr,life[row-2],NUM_COLS/8);
			}
			LED_RED_TOGGLE;
			//break;
			// display life generation and generations/second on LCD
			if (display_results(++generation)) break;
			if (switches) break;
		}
	}
} // end main()

int inline get_neighbors(uint16 row, uint16 col,int col_div8,int col_rem)
{
	register int neighbors = 0;
	register int col_ndiv8 = (col+1) >> 3;
	register int col_pdiv8 = (col-1) >> 3;
	register int col_prem = 0x80 >> ((col-1) & 0x07);
	register int col_nmrem = 0x80 >> ((col+1) & 0x07);
	if((life_cr[col_pdiv8] & col_prem)) neighbors++;
	if((life_cr[col_ndiv8] & col_nmrem)) neighbors++;
	if((life_nr[col_pdiv8] & col_prem)) neighbors++;
	if(life_nr[col_ndiv8] & col_nmrem) neighbors++;
	if(life_nr[col_div8] & col_rem) neighbors++;
	if(life_pr[col_pdiv8] & col_prem) neighbors++;
	if(life_pr[col_ndiv8] & col_nmrem) neighbors++;
	if(life_pr[col_div8] & col_rem) neighbors++;
	return neighbors;
}

int inline get_neighbors_fast(int cr_byte, int nr_byte, int pr_byte, uint16 row, uint16 col,int col_rem,int col_div8)
{
	register int neighbors = 0;
	int col_prem = col_rem << 1;
	int col_nmrem = col_rem >> 1;
	if(cr_byte & col_prem) neighbors++;
	if(cr_byte & col_nmrem) neighbors++;
	if(nr_byte & col_prem) neighbors++;
	if(nr_byte & col_nmrem) neighbors++;
	if(nr_byte & col_rem) neighbors++;
	if(pr_byte & col_prem) neighbors++;
	if(pr_byte & col_nmrem) neighbors++;
	if(pr_byte & col_rem) neighbors++;
	return neighbors;
}

int  inline get_neighbors_prev(int cr_byte, int nr_byte, int pr_byte, uint16 row, uint16 col,int col_rem,int col_div8)
{
	register int neighbors = 0;
	int col_prem = 1;
	int col_nmrem = 64;
	register int col_pdiv8 = col_div8-1;
	if(life_cr[col_pdiv8] & col_prem) neighbors++;
	if(cr_byte & col_nmrem) neighbors++;
	if(life_nr[col_pdiv8] & col_prem) neighbors++;
	if(nr_byte & col_nmrem) neighbors++;
	if(nr_byte & col_rem) neighbors++;
	if(life_pr[col_pdiv8] & col_prem) neighbors++;
	if(pr_byte & col_nmrem) neighbors++;
	if(pr_byte & col_rem) neighbors++;
	return neighbors;
}

int inline get_neighbors_next(int cr_byte, int nr_byte, int pr_byte, uint16 row, uint16 col,int col_rem,int col_div8)
{
	register int neighbors = 0;
	int col_prem = 2;
	int col_nmrem = 128;
	int col_ndiv8 = col_div8 + 1;
	if(cr_byte & col_prem) neighbors++;
	if(life_cr[col_ndiv8] & col_nmrem) neighbors++;
	if(nr_byte & col_prem) neighbors++;
	if(life_nr[col_ndiv8] & col_nmrem) neighbors++;
	if(nr_byte & col_rem) neighbors++;
	if(pr_byte & col_prem) neighbors++;
	if(life_pr[col_ndiv8] & col_nmrem) neighbors++;
	if(pr_byte & col_rem) neighbors++;
	return neighbors;
}

void inline cell_death(uint16 row, uint16 col)
{
	life[(row)][(col) >> 3] &= ~(0x80 >> ((col) & 0x07));
	lcd_point((col) << 1, (row) << 1, 6);
}

void inline cell_birth(uint16 row, uint16 col)
{
	life[(row)][(col) >> 3] |= (0x80 >> ((col) & 0x07));
	lcd_point((col) << 1, (row) << 1, 7);	
}

