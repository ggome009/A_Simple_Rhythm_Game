/*
 * rhythmGame.c
 *
 * Created: 3/12/2018 5:22:18 AM
 * Author : Gabriel Gomez
 */ 

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include "io.c"
#include "timer.h"
#include "bit.h"
#include "piano.h"
#include "songs.h"
#include "pwm.h"
#include "shift.c"

const unsigned char tasksNum = 7;
const unsigned long int tasksPeriodGCD = 5; 
task tasks[7];

#define leftButton  ~PINA & 0x08
#define upButton    ~PINA & 0x04
#define downButton  ~PINA & 0x02
#define rightButton ~PINA & 0x01

typedef enum {False, True} bool;
typedef enum {Divinity, EonBreak} songSelector;
typedef enum {songPlay, highScore} menuOption;

enum lfButtonStates {initLf, pollLf};
enum upButtonStates {initUp, pollUp};
enum dnButtonStates {initDn, pollDn};
enum rtButtonStates {initRt, pollRt};
	
enum LCDStates {initMenu, startScreen, toSongSelect, toSongSelect2, songSelect, toStartScreen, songOptions, toSongOptions, playSong, toPlaySong, countDown, displayScore, backHome, highScoreScreen,  toSongOptions2};

enum noteSetStates {initNoteSet, wait4Play, playNotes};
	
enum LEDMatrixStates {initMatrix, MatrixIdle, MatrixPlay};

//inital values of button inputs
bool LF = False;
bool UP = False;
bool DN = False;
bool RT = False;

//used for keeping track of when new information needs to be output on LCD Screen
//stops the LCD Screen from refreshing constantly
bool ScreenUpdated = False;

//initial values of Menu Options
songSelector currentSong = Divinity;
menuOption songOption = songPlay;

//control signals to let LED Matrix and LCD Screen know when to start and stop play states
bool songEnd = False;
bool play = False;

//used for stores the current note being played when the play state is active
unsigned char noteCount = 0;

//stores the button input desired for the note being played
unsigned char noteTarget = 0x00;

//stores the score of the current game
unsigned short currentScore = 0;

//used for play state to update the score whenever the player's score gets incremented
bool scoreChanged = False;

//song array and max size per song
song songs[2];
#define songSize 128

//high score variables
unsigned char DivinityHighScore = 0;
unsigned char EonBreakHighScore = 0;

//LED Matrix Patterns
unsigned char LFARR[8]={0x00,0x18,0x18,0x18,0x7E,0x3C,0x18,0x00}; //LED Matrix pattern for left arrow
unsigned char RTARR[8]={0x00,0x18,0x3C,0x7E,0x18,0x18,0x18,0x00}; //LED Matrix pattern for right arrow
unsigned char DNARR[8]={0x00,0x10,0x30,0x7E,0x7E,0x30,0x10,0x00}; //LED Matrix pattern for down arrow
unsigned char UPARR[8]={0x00,0x08,0x0C,0x7E,0x7E,0x0C,0x08,0x00}; //LED Matrix pattern for up arrow
unsigned char DISPLAY[8]={0,0,0,0,0,0,0,0};

unsigned char count = 0; //used for countdown state to time down before playing song

int ltButtonTick(int state) {
	switch (state) { //State Transitions
		case initLf:
			state = pollLf;
			break;
		case pollLf:
			state = pollLf;
			break;
		default:
			state = initLf;
			break;
	} // End State Transitions
	
	switch (state) { //State Actions
		case initLf:
			break;
		case pollLf:
			LF = (leftButton) ? True : False;
			break;
	} // End State Actions
	
	return state;
}

int upButtonTick(int state) {
	switch (state) { //State Transitions
		case initUp:
			state = pollUp;
			break;
		case pollUp:
			state = pollUp;
			break;
		default:
			state = initUp;
			break;
	} // End State Transitions
	
	switch (state) { //State Actions
		case initUp:
			break;
		case pollUp:
			UP = (upButton) ? True : False;
			break;
	} // End State Actions
	
	return state;
}

int dnButtonTick(int state) {
	switch (state) { //State Transitions
		case initDn:
			state = pollDn;
			break;
		case pollDn:
			state = pollDn;
			break;
		default:
			state = initDn;
			break;
	} // End State Transitions
	
	switch (state) { //State Actions
		case initDn:
			break;
		case pollLf:
			DN = (downButton) ? True : False;
			break;
	} // End State Actions
	
	return state;
}

int rtButtonTick(int state) {
	switch (state) { //State Transitions
		case initRt:
			state = pollRt;
			break;
		case pollRt:
			state = pollRt;
			break;
		default:
			state = initRt;
			break;
	} // End State Transitions
	
	switch (state) { //State Actions
		case initRt:
			break;
		case pollRt:
			RT = (rightButton) ? True : False;
			break;
	} // End State Actions
	
	return state;
}

int LCDTick(int state) {

	switch (state) { // State Transitions
		case initMenu:
			state = startScreen;
			break;;
		case startScreen:
			if(!LF && !UP && !DN && RT) {
				state = toSongSelect;
			} else {
				state = startScreen;
				ScreenUpdated = True;
			}
			break;
		case toSongSelect:
			if(!LF && !UP && !DN && !RT) {
				state = songSelect;
				LCD_ClearScreen();
				ScreenUpdated = False;
			} else if (!LF && !UP && !DN && RT) {
				state = toSongSelect;
			} else {
				state = startScreen;
			}
			break;
		case toSongSelect2:
			if(!LF && !UP && !DN && !RT) {
				state = songSelect;
				LCD_ClearScreen();
				ScreenUpdated = False;
			} else if (LF && !UP && !DN && !RT) {
				state = toSongSelect2;
			} else {
				state = songOptions;
			}
			break;
		case songSelect:
			if(LF && !UP && !DN && !RT) {
				state = toStartScreen;
			} else if (!LF && !UP && !DN && RT) {
				state = toSongOptions;
			} else if (!LF && !UP && DN && !RT && (currentSong != EonBreak)) {
				currentSong = EonBreak;
				state = songSelect;
				ScreenUpdated = False;
			} else if (!LF && UP && !DN && !RT && (currentSong != Divinity)) {
				currentSong = Divinity;
				state = songSelect;
				ScreenUpdated = False;
			}
			break;
		case toStartScreen:
			if(!LF && !UP && !DN && !RT) {
				state = startScreen;
				LCD_ClearScreen();
				ScreenUpdated = False; 
			} else if(LF && !UP && !DN && !RT) {
				state = toStartScreen;
			} else {
				state = songSelect;
			}
			break;
		case songOptions:
			if(LF && !UP && !DN && !RT) {
				state = toSongSelect2;
			} else if (!LF && !UP && !DN && RT) {
				LCD_ClearScreen();
				if(songOption == highScore) {
					state = highScoreScreen;
					LCD_DisplayString1Line(1, 1, "High Score:00000");
				} else if (songOption == songPlay) {
					state = toPlaySong;
					PWM_on();
					LCD_DisplayString(1, "Score: 00000");
				}
			} else if (!LF && !UP && DN && !RT && (songOption != highScore)) {
				songOption = highScore;
				state = songOptions;
				ScreenUpdated = False;
			} else if (!LF && UP && !DN && !RT && (songOption != songPlay)) {
				songOption = songPlay;
				state = songOptions;
				ScreenUpdated = False;
			}
			break;
		case toSongOptions:
			if(!LF && !UP && !DN && !RT) {
				state = songOptions;
				LCD_ClearScreen();
				ScreenUpdated = False; 
			} else if(!LF && !UP && !DN && RT) {
				state = toSongOptions;
			} else {
				state = songSelect;
			}
			break;
		case playSong:
			if(songEnd || !play) {
				ScreenUpdated = False;
				state = displayScore;
				PWM_off();
				noteCount = 0;
			} else {
				state = playSong;
			}
			break;
		case toPlaySong:
			if(!LF && !UP && !DN && !RT) {
				state = countDown;
			} else if (!LF && !UP && !DN && RT) {
				state = toPlaySong;
			} else {
				state = songOptions;
			}
			break;
		case countDown:
			if(count < 50) {
				++count;
				state = countDown;
			} else {
				count = 0;
				state = playSong;
				play = True;
			}
			break;
		case displayScore:
			if(LF || UP || DN || RT) {
				state = backHome;
			}
			break;
		case backHome:
			if(!LF && !UP && !DN && !RT) {
				state = startScreen;
				ScreenUpdated = False;
				currentScore = 0;
				LCD_ClearScreen();
			} else {
				state = backHome;
			}
			break;
		case highScoreScreen:
			if(LF && !UP && !DN && !RT) {
				state = toSongOptions2;
			} else {
				state = highScoreScreen;
			}
			break;
		case toSongOptions2:
			if(!LF && !UP && !DN && !RT) {
				state = songOptions;
				LCD_ClearScreen();
				ScreenUpdated = False; 
			} else if(LF && !UP && !DN && !RT) {
				state = toSongOptions2;
			} else {
				state = highScoreScreen;
			}
			break;
		default:
			state = initMenu;
			break;
	} // End State Transitions
	
	switch (state) { // State Actions
		case initMenu:
			break;
		case startScreen:
			if(ScreenUpdated) {}
			else {
				delay_ms(30);
				LCD_Cursor(5);
				
				LCD_WriteData(0x00);
				LCD_WriteData(0x01);
				LCD_WriteData(0x02);
				LCD_WriteData(0x03);
				LCD_WriteData(0x04);
				LCD_WriteData(0x02);
				LCD_WriteData(0x01);
				LCD_WriteData(0x05);
				
				LCD_DisplayString1Line(1, 2, "Press _ to start");
				LCD_Cursor(7 + 16);
				LCD_WriteData(0x7E);
				LCD_Cursor(33);
				ScreenUpdated = True;
			}
			break;
		case toSongSelect:
			break;
		case toSongSelect2:
			break;
		case songSelect:
			if(ScreenUpdated) {}
			else {
				LCD_DisplayString1Line(1, 1, "  Divinity");
				LCD_DisplayString1Line(1, 2, "  Eon Break");
				if (currentSong == Divinity) {
					LCD_Cursor(1);	
				} else {
					LCD_Cursor(17);

				}
				LCD_WriteData(0x02);
				LCD_WriteData(' ');
				LCD_Cursor(33);
				ScreenUpdated = True;
			}
			break;
		case toStartScreen:
			break;
		case songOptions:
			if(ScreenUpdated) {}
			else {
				LCD_DisplayString1Line(1, 1, "  Play song");
				LCD_DisplayString1Line(1, 2, "  See High Score");
				if (songOption == songPlay) {
					LCD_Cursor(1);
				} else {
					LCD_Cursor(17);
				}
				LCD_WriteData(0x02);
				LCD_WriteData(' ');
				LCD_Cursor(33);
				ScreenUpdated = True;
			}
			break;
		case toSongOptions:
			break;
		case playSong:
				if(scoreChanged) {
					LCD_Cursor(8);
					LCD_WriteData('0' + (currentScore / 100) );
					LCD_WriteData('0' + ((currentScore - (currentScore / 100)*100) / 10 ) );
					LCD_WriteData('0' + ((currentScore - (currentScore / 100)*100) % 10 ));
				}
				
			break;
		case toPlaySong:
			break;
		case countDown:
			break;
		case displayScore:
			if(ScreenUpdated) {}	
			else{
				if(currentSong == Divinity && DivinityHighScore < currentScore) {
					DivinityHighScore = currentScore;
					LCD_DisplayString1Line(1, 2, "High Score!");
					eeprom_update_byte((uint8_t*) 1, (uint8_t) currentScore);
				} else if(currentSong == EonBreak && EonBreakHighScore < currentScore) {
					EonBreakHighScore = currentScore;
					LCD_DisplayString1Line(1, 2, "High Score!");
					eeprom_update_byte((uint8_t*) 4, (uint8_t) currentScore);
				}
				ScreenUpdated = True;
			}
			break;
		case backHome:
			break;
		case highScoreScreen:
				LCD_Cursor(12);
				if(currentSong == Divinity){
					LCD_WriteData('0' + (DivinityHighScore / 100) );			
					LCD_WriteData('0' + ((DivinityHighScore - (DivinityHighScore / 100)*100) / 10 ) );
					LCD_WriteData('0' + ((DivinityHighScore - (DivinityHighScore / 100)*100) % 10 ));
				} else if (currentSong == EonBreak) {
					LCD_WriteData('0' + (EonBreakHighScore / 100) );
					LCD_WriteData('0' + ((EonBreakHighScore - (EonBreakHighScore / 100)*100) / 10 ) );
					LCD_WriteData('0' + ((EonBreakHighScore - (EonBreakHighScore / 100)*100) % 10 ));
				}
				LCD_Cursor(33);
			break;
		case toSongOptions2:
			break;
	} // End State Actions
	
	return state;
}

int LEDMatrixTick(int state) {
	static unsigned char i;
	static unsigned char j;
	switch(state) { // State Transitions
		case initMatrix:
			state = MatrixIdle;
			break;
		case MatrixIdle:
			state = play ? MatrixPlay : MatrixIdle;
			break;
		case MatrixPlay:
			state = songEnd ? MatrixIdle : MatrixPlay;
			break;
		default:
			state = initMatrix;
			break;
	} // End State Transitions
	switch(state) { // State Actions
		case initMatrix:
			break;
		case MatrixIdle:
			i = 0;
			j = 0;
			if(!LF && !UP && !DN && RT) {
				for(unsigned char i=0;i<8;i++) {
					ShRegWrite(128>>i);
					ShRegWrite(~RTARR[i]);   
					delay_ms(5);
				}
			} else if (LF && !UP && !DN && !RT) {
				for(unsigned char i=0;i<8;i++) {
					ShRegWrite(128>>i);
					ShRegWrite(~LFARR[i]);   
					delay_ms(5);
				}
			} else if (!LF && !UP && DN && !RT) {
				for(unsigned char i=0;i<8;i++) {
					ShRegWrite(128>>i);
					ShRegWrite(~DNARR[i]);   
					delay_ms(5);
				}
			}  else if (!LF && UP && !DN && !RT) {
				for(unsigned char i=0;i<8;i++) {
					ShRegWrite(128>>i);
					ShRegWrite(~UPARR[i]);   
					delay_ms(5);
				}
			} else {
				ShRegWrite(0);
				ShRegWrite(0);
			}

			break;
		case MatrixPlay:
			for(unsigned char k = 0; k < 8;k++) {
				DISPLAY[k] <<= 1;
			}
			
			if (i < songs[currentSong].times[noteCount]) {
				unsigned char noteTarget = songs[currentSong].press[noteCount];
				
				if (noteTarget == 1) {
					DISPLAY[0] |= 1;
					DISPLAY[1] |= 1;
				} else if (noteTarget == 2) {
					DISPLAY[2] |= 1;
					DISPLAY[3] |= 1;
				} else if (noteTarget == 4) {
					DISPLAY[4] |= 1;
					DISPLAY[5] |= 1;
				} else if (noteTarget == 8) {
					DISPLAY[6] |= 1;
					DISPLAY[7] |= 1;
				}
				++i;
			} else {
				
				if (j < songs[currentSong].rests[noteCount]) {
					
					
					++j;
				} else {
					i = 0;
					j = 0;
				}
			}
			
			for(unsigned char i=0;i<8;i++) {
				ShRegWrite(128>>i);
				ShRegWrite(~DISPLAY[i]);
				delay_ms(5);
			}
			
			
			break;
	}	// End State Actions
	
	
	return state;
}

int noteSetTick(int state) {
	static unsigned char i;
	static unsigned char j;
	unsigned char gameInput = (LF << 3) | (UP << 2) | (DN << 1) | RT;

	switch (state) { //State Transitions
		case initNoteSet:
			state = wait4Play;
			break;
		case wait4Play:
			songEnd = False;
			state = play ? playNotes : wait4Play;
			break;
		case playNotes:
			state = songEnd ? wait4Play : playNotes;
			break;
		default:
			state = initNoteSet;
			break;
	} //State Transitions
	switch (state) { //State Actions
		case initNoteSet:
		case wait4Play:
			i = 0;
			j = 0;
			break;
		case playNotes:
		
			if (i < songs[currentSong].times[noteCount]) {
				noteTarget = songs[currentSong].press[noteCount];
				set_PWM(songs[currentSong].notes[noteCount]);
				
				if(gameInput == noteTarget) {
					currentScore++;
					scoreChanged = True;
				} else {
					scoreChanged = False;
				}
				
				++i;
			} else {
				noteTarget = 0x00;
				if (j < songs[currentSong].rests[noteCount]) {
					set_PWM(0);
					++j;
				} else {
					i = 0;
					j = 0;
					if (noteCount < (unsigned)(char) songSize) {
						noteCount++;
					} else {
						songEnd = True;
						play = False;
						break;
					}
				}
			}
			break;
	} // State Actions
	
	return state;
}

//song information arrays
//Divinity by Porter Robinson- SIZE:73
double notesDIV[songSize] =        { 0, A4,FS4,FS4,FS4,FS4,FS4,FS4,FS4,FS4,FS4,A4,D4,D4,D4,D4,D4,D4,D4,A4,FS4,FS4,FS4,FS4,FS4,FS4,FS4,FS4,FS4,CS4,D4,D4,D4,D4,D4,D4,D4,A4,FS4,FS4,FS4,FS4,FS4,FS4,FS4,FS4,FS4,A4,D4,D4,D4,D4,D4,D4,D4,A4,FS4,FS4,FS4,FS4,FS4,CS5,D5,B4,FS4,B4,A4,A4,A4,A4,A4,A4, 0};
unsigned char timesDIV[songSize] = {16,  4,  4,  2,  4,  2,  2,  4,  2,  2,  2, 4, 4, 2, 4, 2, 2, 4, 8, 4,  4,  2,  4,  2,  2,  4,  2,  2,  2,  4, 4, 2, 4, 2, 2, 4, 8, 4,  4,  2,  4,  2,  2,  4,  2,  2,  2, 4, 4, 2, 4, 2, 2, 4, 8, 4,  4,  2,  4,  2,  2,  4, 4, 4,  4, 4, 4, 2, 4, 2, 2, 4, 0};
unsigned char restsDIV[songSize] = {16,  4,  9,  2,  8,  1,  2,  4,  2,  2,  2, 4, 8, 2, 8, 2, 2, 9, 0, 4,  8,  2,  9,  2,  2,  4,  2,  2,  2,  4, 8, 2, 8, 2, 2, 9, 0, 4,  9,  2,  8,  1,  2,  4,  2,  2,  2, 4, 8, 2, 8, 2, 2, 9, 0, 4,  9,  2,  8,  1,  2,  4, 0, 0,  0, 4, 8, 2, 8, 2, 2, 0,32};
unsigned char pressDIV[songSize] = {16,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2, 1, 4, 4, 4, 4, 4, 4, 4, 1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  8, 4, 4, 4, 4, 4, 4, 4, 1,  2,  2,  2,  2,  2,  2,  2,  2,  2, 1, 4, 4, 4, 4, 4, 4, 4, 1,  2,  2,  2,  2,  2,  1, 2, 4,  8, 4, 8, 8, 8, 8, 8, 8,240};

//Eon Break by Virtual Self - SIZE:94
double notesEON[songSize] =        { 0,A3,C4,E4,G4,A4,A4,A4,A4,G4,G4,G4,G4,A4,E4,E4,E4,E4,C5,B4,G4,E4,D4,B3,C5,B4,G4,E4,D4,B3,D4,G4,E4,E4,E4,E4,D4,D4,D4,C4,B3,A3,A3,A3,A3,A3,A3,A3,C4,E4,G4,A4,A4,A4,A4,G4,G4,G4,G4,A4,E4,E4,E4,E4,C5,B4,G4,E4,D4,B3,C5,B4,G4,E4,D4,B3,D4,G4,E4,E4,E4,E4,D4,D4,D4,E4,C4,B3,A3,A3,A3,A3,A3, 0};
unsigned char timesEON[songSize] = {16, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0};
unsigned char restsEON[songSize] = {16, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4,12,32};
unsigned char pressEON[songSize] = {16, 8, 4, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 4, 4, 4, 4, 1, 1, 2, 4, 8, 8, 1, 1, 2, 4, 8, 8, 4, 1, 2, 2, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8, 4, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 4, 4, 4, 4, 1, 1, 2, 4, 8, 8, 1, 1, 2, 4, 8, 8, 4, 1, 2, 2, 2, 2, 4, 4, 4, 1, 2, 4, 8, 8, 8, 8, 8,240};


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; //INPUT
	DDRB = 0xFF; PORTB = 0x00; //OUTPUT
    DDRC = 0xFF; PORTC = 0x00; //OUTPUT
	DDRD = 0xFF; PORTD = 0x00; //OUTPUT
	
	LCD_init();
	ShRegInit();
	
	//flash LED Matrix to test if it is working
	//all LEDS should light up then power off immediately after
	ShRegWrite(0);
	ShRegWrite(255);
	
	//initialize EEPROM addresses to 0 
	//used for storing high score values
	if(eeprom_read_byte((uint8_t*)1) == 255) {
		eeprom_update_byte((uint8_t*)1, (uint8_t) 0);
	}
	if(eeprom_read_byte((uint8_t*)4) == 255) {
		eeprom_update_byte((uint8_t*)4, (uint8_t) 0);
	}
	
	DivinityHighScore = eeprom_read_byte((uint8_t*)1);
	EonBreakHighScore = eeprom_read_byte((uint8_t*)4);
	
	//build songs array
	static unsigned char k = 0;
	for(k = 0; k < 73; ++k) {
		songs[0].notes[k] = notesDIV[k];
		songs[0].times[k] = timesDIV[k];
		songs[0].rests[k] = restsDIV[k];
		songs[0].press[k] = pressDIV[k];
	}
	
	for(k = 0; k < 94; ++k) {
		songs[1].notes[k] = notesEON[k];
		songs[1].times[k] = timesEON[k];
		songs[1].rests[k] = restsEON[k];
		songs[1].press[k] = pressEON[k];
	}
	
	//build tasks array
	unsigned char i = 0;
	tasks[i].state = -1;
	tasks[i].period = 40;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &ltButtonTick;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 40;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &upButtonTick;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 40;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &dnButtonTick;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 40;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &rtButtonTick;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 40;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &LCDTick;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 15;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &noteSetTick;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 10;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &LEDMatrixTick;
	i++;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
    while (1) {
		sleep_mode();
	}
		
		
	
}
