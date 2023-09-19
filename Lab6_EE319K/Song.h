// Song.h
// Lab 6 Extra credit 1) 
// playing your favorite song.
//
// For use with the TM4C123
// ECE319K lab6 extra credit
// Program written by: put your names here
// 1/2/23


// Description: 
// This file contains the interrupt delays for notes, the samples
// for the sine wave used to make sound, and the data structure
// containing notes for your favorite.
//

#ifndef SONG_H
#define SONG_H
void Song_Init(void);
// Play your favorite song, while button pushed or until end
void Song_Play(void);

// Stop song
void Song_Stop(void);

#endif

