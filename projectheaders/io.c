#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "io.h"

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))
          
/*-------------------------------------------------------------------------*/

#define DATA_BUS PORTC		// port connected to pins 7-14 of LCD display
#define CONTROL_BUS PORTD	// port connected to pins 4 and 6 of LCD disp.
#define RS 6			// pin number of uC connected to pin 4 of LCD disp.
#define E 7			// pin number of uC connected to pin 6 of LCD disp.

/*-------------------------------------------------------------------------*/


unsigned char portrOPN[8] = {15, 14, 12, 12, 12, 12, 14, 15}; 
	// _1111
	// _111_
	// _11__
	// _11__
	// _11__
	// _11__
	// _111_
	// _1111
unsigned char portrEQL[8] = { 0,  0,  0, 31,  0, 31,  0,  0};
unsigned char portrEYE[8] = { 0,  0,  4, 10, 21, 10,  4,  0};
unsigned char portrMTL[8] = { 0,  0,  1,  3,  6, 12,  8,  0};
unsigned char portrMTR[8] = { 0,  0, 16, 24, 12,  6,  2,  0};
unsigned char portrCLS[8] = {30, 14,  6,  6,  6,  6, 14, 30};

void LCD_init(void) {

    //wait for 100 ms.
	delay_ms(100);
	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0F);
	LCD_WriteCommand(0x01);
	delay_ms(10);		
	
	LCD_BuildCharacter(0, portrOPN); // stores porter open brace to memory location 0x00
	LCD_BuildCharacter(1, portrEQL); // stores porter equal sign to memory location 0x01
	LCD_BuildCharacter(2, portrEYE); // stores porter eyeball to memory location 0x02
	LCD_BuildCharacter(3, portrMTL); // stores 1st half of porter mouth to location 0x03
	LCD_BuildCharacter(4, portrMTR); // stores 2nd half of porter mouth to location 0x04
	LCD_BuildCharacter(5, portrCLS); // stores porter close brace to mem location 0x05
}

void LCD_ClearScreen(void) {
	LCD_WriteCommand(0x01);
}

void LCD_WriteCommand (unsigned char Command) {
   CLR_BIT(CONTROL_BUS,RS);
   DATA_BUS = Command;
   SET_BIT(CONTROL_BUS,E);
   asm("nop");
   CLR_BIT(CONTROL_BUS,E);
   delay_ms(2); // ClearScreen requires 1.52ms to execute
}

void LCD_WriteData(unsigned char Data) {
   SET_BIT(CONTROL_BUS,RS);
   DATA_BUS = Data;
   SET_BIT(CONTROL_BUS,E);
   asm("nop");
   CLR_BIT(CONTROL_BUS,E);
   delay_ms(1);
}

void LCD_DisplayString( unsigned char column, const unsigned char* string) {
   LCD_ClearScreen();
   unsigned char c = column;
   while(*string) {
      LCD_Cursor(c++);
      LCD_WriteData(*string++);
   }
}


//same as LCD_DisplayString except it doesn't clear the screen, only writes to one line of the display
void LCD_DisplayString1Line( unsigned char column, unsigned char line, const unsigned char* string) {
	!(line - 1) ? LCD_Cursor(1) : LCD_Cursor(17);
	
	for(unsigned char i = 0; i < 17; i++){
		LCD_WriteData(' ');
	}
	
	
	unsigned char c = (line - 1) ? column + 16 : column;
	
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}

void LCD_BuildCharacter(unsigned char location, unsigned char *p) {
	unsigned char i;
	if(location < 8) {
		LCD_WriteCommand(0x40+(location*8));
		for(i = 0; i < 8; i++) {
			LCD_WriteData(p[i]);
		}
	}
	
	LCD_WriteCommand(0x80);
}

void LCD_Cursor(unsigned char column) {
   if ( column < 17 ) { // 16x1 LCD: column < 9
						// 16x2 LCD: column < 17
      LCD_WriteCommand(0x80 + column - 1);
   } else {
      LCD_WriteCommand(0xB8 + column - 9);	// 16x1 LCD: column - 1
											// 16x2 LCD: column - 9
   }
}

void delay_ms(int miliSec) //for 8 Mhz crystal
{
    int i,j;
    for(i=0;i<miliSec;i++)
    for(j=0;j<775;j++)
  {
   asm("nop");
  }
}
