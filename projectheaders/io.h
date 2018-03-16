#ifndef __io_h__
#define __io_h__

void LCD_init();
void LCD_ClearScreen(void);
void LCD_WriteCommand (unsigned char Command);
void LCD_Cursor (unsigned char column);
void LCD_DisplayString(unsigned char column ,const unsigned char *string);
void LCD_DisplayString1Line( unsigned char column, unsigned char line, const unsigned char* string);
void LCD_BuildCharacter(unsigned char location, unsigned char *p);
void delay_ms(int miliSec);
#endif

