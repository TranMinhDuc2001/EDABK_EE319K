// Sound.c
// This module contains the SysTick ISR that plays sound
// Runs on TM4C123
// Program written by: put your names here
// Date Created: 3/6/17 
// Last Modified: 1/2/23 
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "../inc/dac.h"
#include "../inc/tm4c123gh6pm.h"

const uint8_t SineWave[128] = {31,33,34,36,37,39,40,42,43,45,46,48,49,50,51,53,54,55,56,57,
															 58,59,59,60,61,61,62,62,62,63,63,63,63,63,63,63,62,62,61,61,
															 60,60,59,58,57,56,55,54,53,52,51,50,48,47,45,44,43,41,40,38,
															 36,35,33,32,30,29,27,25,24,22,21,19,18,17,15,14,12,11,10,9,
															 8,7,6,5,4,3,2,2,2,1,1,1,0,0,0,0,0,0,0,
															 1,1,1,2,2,3,3,4,5,6,7,8,9,11,12,13,14,16,17,19,
															 20,22,23,25,26,28,29,30,31};
															 
uint8_t Index=0;           // Index varies from 0 to 15
// **************Sound_Init*********************
// Initialize digital outputs and SysTick timer
// Called once, with sound/interrupts initially off
// Input: none
// Output: none
void Sound_Init(void){
	
	DAC_Init();

  GPIO_PORTB_DIR_R |= 0x20;     // make PB5 out

  GPIO_PORTB_DR8R_R |= 0x20;    // can drive up to 8mA out

  GPIO_PORTB_DEN_R |= 0x20;     // enable digital I/O on PB5

  NVIC_ST_CTRL_R = 0;           // disable SysTick during setup

  NVIC_ST_RELOAD_R = 124;     // reload value for 500us (assuming 40MHz)

  NVIC_ST_CURRENT_R = 0;        // any write to current clears it

  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0               

  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts

  
}

// **************Sound_Start*********************
// Start sound output, and set Systick interrupt period 
// Sound continues until Sound_Start called again, or Sound_Off is called
// This function returns right away and sound is produced using a periodic interrupt
// Input: interrupt period
//           Units of period to be determined by YOU
//           Maximum period to be determined by YOU
//           Minimum period to be determined by YOU
//         if period equals zero, disable sound output
// Output: none
// period = 500us.f
void Sound_Start(uint32_t period){
	
	DAC_Init();
	
	GPIO_PORTB_DIR_R |= 0x20;     // make PB5 out

  GPIO_PORTB_DR8R_R |= 0x20;    // can drive up to 8mA out

  GPIO_PORTB_DEN_R |= 0x20;     // enable digital I/O on PB5

  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup

  NVIC_ST_RELOAD_R = period-1;// reload value

  NVIC_ST_CURRENT_R = 0;      // any write to current clears it

  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0  

  NVIC_ST_CTRL_R = 0x0007; // enable SysTick with core clock and interrupts
}

// **************Sound_Voice*********************
// Change voice
// EE319K optional
// Input: voice specifies which waveform to play
//           Pointer to wave table
// Output: none
void Sound_Voice(const uint8_t *voice){
  // optional
}
// **************Sound_Off*********************
// stop outputing to DAC
// Output: none
void Sound_Off(void){
 

}
// **************Sound_GetVoice*********************
// Read the current voice
// EE319K optional
// Input: 
// Output: voice specifies which waveform to play
//           Pointer to current wavetable
const uint8_t *Sound_GetVoice(void){
  // if needed
  return 0; // replace
}
#define PF4 (*((volatile uint32_t *)0x40025040))
#define PF3 (*((volatile uint32_t *)0x40025020))
#define PF2 (*((volatile uint32_t *)0x40025010))
#define PF1 (*((volatile uint32_t *)0x40025008))
#define PF0 (*((volatile uint32_t *)0x40025004))

// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
	
	Index = (Index+1)% 128;

  DAC_Out(SineWave[Index]);
}



