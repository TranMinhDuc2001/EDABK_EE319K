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

const uint8_t SineWave[126] = {32,38,38,44,45,50,50,51,52,53,54,55,55,56,57,58,58,58,59,59,60,60,60,61,61,61,62,62,62,63,63,63,
															 63,63,63,62,62,62,61,61,61,60,60,60,59,59,58,58,58,57,56,55,55,54,53,52,51,50,50,45,44,38,38,32,
															 32,26,26,22,21,16,16,15,14,13,12,11,11,10,9,8,8,8,7,7,6,6,6,4,4,2,2,2,1,1,1,
															 1,1,1,2,2,2,4,4,6,6,6,7,7,8,8,8,9,10,11,11,12,13,14,15,16,16,21,22,26,26,32};
	
	
	
																//32,32,31,31,31,30,30,30,29,29,28,28,28,27,27,26,26,26,25,24,23,23,22,21,20,19,19,14,9,4,1,
uint8_t Index=0;           // Index varies from 0 to 15
// **************Sound_Init*********************
// Initialize digital outputs and SysTick timer
// Called once, with sound/interrupts initially off
// Input: none
// Output: none
void Sound_Init(void){
  uint32_t volatile delay;
	
	DAC_Init();

  SYSCTL_RCGCGPIO_R |= 0x00000002; // activate port B

  delay = SYSCTL_RCGCGPIO_R;

  GPIO_PORTB_DIR_R |= 0x20;     // make PB5 out

  GPIO_PORTB_DR8R_R |= 0x20;    // can drive up to 8mA out

  GPIO_PORTB_DEN_R |= 0x20;     // enable digital I/O on PB5

  NVIC_ST_CTRL_R = 0;           // disable SysTick during setup

  NVIC_ST_RELOAD_R = 39999;     // reload value for 500us (assuming 80MHz)

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
	
	Index = 0;

  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup

  NVIC_ST_RELOAD_R = period-1;// reload value

  NVIC_ST_CURRENT_R = 0;      // any write to current clears it

  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1

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

	
	Index = (Index+1)%126;

  DAC_Out(SineWave[Index]);
}



