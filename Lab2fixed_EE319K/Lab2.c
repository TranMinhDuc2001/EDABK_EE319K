// **************** Lab2.c *************
// Program written by: put your names here
// Date Created: 1/18/2017 
// Last Modified: 12/31/2022 
#include "Lab2.h"
// Put your name and EID in the next two lines
char Name[] = "duc";
char EID[] = "20203370";

#include <stdint.h>
#include <stdlib.h>

// Inputs: x1,y1 is the first point
//         x2,y2 is the second point
// Output: calculate distance
// see UART window to see if you have square, Manhattan or ECE319K distance
// The input/output values will be displayed in the UART window
int32_t Distance(int32_t x1, int32_t y1, int32_t x2, int32_t y2){
// Replace this following line with your solution
	int32_t x;
	int32_t y;
	int32_t Manhattan_distance;
	x = abs(x1 - x2);
	y = abs(y1 - y2);
	Manhattan_distance = x + y;
	
  return Manhattan_distance;
}

// Inputs: rect1 is x1,y1,w1,h1 the first rectangle
//         rect2 is x2,y2,w2,h2 the second rectangle
// Output: 1 if the rectangles overlap
//         0 if the rectances do not overlap
// Notes: x1 is rect1[0]  x2 is rect2[0]
//        y1 is rect1[1]  y2 is rect2[1]
//        w1 is rect1[2]  w2 is rect2[2]
//        h1 is rect1[3]  h2 is rect2[3]
// The input/output values will be displayed in the UART window
int32_t OverLap(int32_t rect1[4], int32_t rect2[4]){
// Replace this following line with your solution
	int32_t x11 = rect1[0];
	int32_t y11 = rect1[1];
	int32_t w1 = rect1[2];
	int32_t h1 = rect1[3];
	
	int32_t x22 = rect2[0];
	int32_t y22 = rect2[1];
	int32_t w2 = rect2[2];
	int32_t h2 = rect2[3];
	
	int32_t x12,y12,x21,y21;
	x12 = x11+w1-1;
	y12 = y11-h1+1;
	x21 = x22+w2-1;
	y21 = y22-h2+1;
	
	if(x12 < x22 || x21<x11){
		return 0;// khong over lap
	}
	if(y12>y22 || y21>y11){
		return 0;// khong over lap
	}
  return 1;//over lap
}
// do not edit this 2-D array
const char Hello[4][8] ={
   "Hello  ",    // language 0
   "\xADHola! ", // language 1
   "Ol\xA0    ", // language 2
   "All\x83   "  // language 3
};
// Hello[0][0] is 'H'
// Hello[0][1] is 'e'
// Hello[0][2] is 'l'
// Hello[0][3] is 'l'
// Hello[0][4] is 'o'
// Hello[1][0] is 0xAD
// Hello[1][1] is 'H'
// Hello[1][2] is 'o'
// Hello[1][3] is 'l'
// Hello[1][4] is 'a'
// Hello[1][5] is '!'
void LCD_OutChar(char letter);

// Print 7 characters of the hello message
// Inputs: language 0 to 3
// Output: none
// You should call LCD_OuChar exactly 7 times

void SayHello(uint32_t language){
// Replace this following line with your solution
	for(int i = 0; i < 7; i++){
		LCD_OutChar(Hello[language][i]);
	}
}