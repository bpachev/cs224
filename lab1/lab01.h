/*
 * lab01.h
 *
 *  Created on: May 16, 2014
 *      Author: proper
 */

#ifndef LAB01_H_
#define LAB01_H_

// defined constants -----------------------------------------------------------
#define myCLOCK			1200000			// 1.2 Mhz clock
#define	WDT_CTL			WDT_MDLY_32		// WD configuration (SMCLK, ~32 ms)
#define WDT_CPI			32000			// WDT Clocks Per Interrupt (@1 Mhz)
#define	WDT_1SEC_CNT	myCLOCK/WDT_CPI	// WDT counts/second

//#define TERMINAL(f) sprintf(buffer,"\n\r%d. ",count++);print(buffer);sprintf(buffer,f);print(buffer);
#define TERMINAL print
#define TERMINAL1(f,x) sprintf(buffer,"\n\r%d. ",count++);print(buffer);sprintf(buffer,f,x);print(buffer);
#define TERMINAL2(f,x) sprintf(buffer,"\n\r%d. ",count++);print(buffer);sprintf(buffer,f,x,x);print(buffer);
#define TERMINAL3(f,x,y) sprintf(buffer,"\n\r%d. ",count++);print(buffer);sprintf(buffer,f,x,y);print(buffer);

#ifndef count
extern unsigned int count;
extern char buffer[];
#endif

void lab01_init();
void print(const char *s);

#endif /* LAB01_H_ */
