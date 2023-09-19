// Music.h
// This program contains timer0A timer1A ISR for 
// playing your favorite Song.
//
// For use with the TM4C123
// EE319K lab6 
// 10/15/21

// Description: 
// This file contains the interrupt delays for notes, the samples
// for the sine wave used to make sound, and the data structure
// containing notes for your favorite.
//

#ifndef MUSIC_H
#define MUSIC_H

// Play your favorite song, while button pushed or until end
void Music_PlaySong(void);

// Stop song
void Music_StopSong(void);

#endif

