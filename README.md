# rhythmGame_120bproject
A simple rhythm game programmable to the Atmel ATmega1284 microcontroller.

## Introduction
Players earn points by hitting buttons that correspond to the notes that come down the note highway.
The goal is to accumulate the most points by the end of the song.
This project was a lot of fun to do because it's a perfect blend of 3 things that I love: video games, music, and computer science.

![alt text](https://i.imgur.com/yzIirfA.png)

An overview video of this project can be found [here](https://www.youtube.com/watch?v=D_F-ub7Tf_0)

## Hardware
### Parts List
* ATmega1284 microcontroller
* 8x8 LED matrix
* 2 shift registers
* 16x2 LCD display
* 4 Sanwa arcade buttons
* piezo buzzer
* a lot of jumper wire
* 8 330 Ohm resistors

## Known Bugs
* After completing a game, sometimes the score will increase by 16
* * I believe this has something to do with the song struct

