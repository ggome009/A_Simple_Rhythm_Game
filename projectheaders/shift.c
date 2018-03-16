#include <avr/io.h>
#include <util/delay.h>

#include "io.h"
#include <util/delay_basic.h>

//BIT OPERATIONS
#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))

//CONNECTIONS
#define SHIFT_PORT	PORTD
#define SHIFT_DDR	DDRD

#define DATA_LINE 4		//Data pin location

#define SHIFT_LINE 2	//Shift Clock pin location 
#define STORE_LINE 3	//Store Clock pin location


//Initialize Shift Register
void ShRegInit() {
	// Data, Shift clock, Store Clock output
	SET_BIT(SHIFT_DDR,SHIFT_LINE);
	SET_BIT(SHIFT_DDR,STORE_LINE);
	SET_BIT(SHIFT_DDR,DATA_LINE);
}

//Set Data line high
void ShRegDataHigh() {
	SET_BIT(SHIFT_PORT,DATA_LINE);
}

//Set Data line low
void ShRegDataLow() {
	CLR_BIT(SHIFT_PORT,DATA_LINE);
}

//Pulse Shift Clock
void ShRegPulse() {
	SET_BIT(SHIFT_PORT,SHIFT_LINE);
	CLR_BIT(SHIFT_PORT,SHIFT_LINE);
}

//Pulse Store Clock
void ShRegLatch() {
   SET_BIT(SHIFT_PORT,STORE_LINE);
   _delay_loop_1(1);

   CLR_BIT(SHIFT_PORT,STORE_LINE);
   _delay_loop_1(1);
}


void ShRegWrite(unsigned char data) {
	for(unsigned char i=0;i<8;i++) {
		if(data & 0x80) {
			ShRegDataHigh();
		} else {
			ShRegDataLow();
		}

		ShRegPulse();
		data=data<<1;
	}

	ShRegLatch();
}
