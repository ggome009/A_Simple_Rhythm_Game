#ifndef __songs_h__
#define __songs_h__

typedef struct song {
	double notes[songSize];
	unsigned char times[songSize];
	unsigned char rests[songSize];
	unsigned char press[songSize];
} song;

#endif