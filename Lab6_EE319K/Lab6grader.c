// Lab6grader.c
// Runs on TM4C123
// Periodic timer interrupt data collection
// PLL turned on at 80 MHz
// Implements Logic Analyzer on Port B, D, or E
//

// Jonathan Valvano. Daniel Valvano
// 1/2/2023

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2021

 Copyright 2023 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include <string.h>
#include "UART.h"
#include "Timer3A.h"
#include "../inc/tm4c123gh6pm.h"
#include "Lab6Grader.h"


// Timer5A periodic interrupt implements logic analyzer
// 0 for scope
void Scope(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
// 1 for PA7-2 logic analyzer A
#define PA72       (*((volatile uint32_t *)0x400043F0))
void LogicAnalyzerA(void){  // called 10k/sec
  UART0_DR_R = (PA72>>2)|0x80;   // send data to TExaSdisplay
}
// 2 for PB6-0 logic analyzer B
#define PB60       (*((volatile uint32_t *)0x400051FC))
void LogicAnalyzerB(void){  // called 10k/sec
  UART0_DR_R = PB60|0x80;   // send data to TExaSdisplay
}
// 3 for PC7-4 logic analyzer C
#define PC74       (*((volatile uint32_t *)0x400063C0))
void LogicAnalyzerC(void){  // called 10k/sec
  UART0_DR_R = (PC74>>4)|0x80;   // send data to TExaSdisplay
}
// 4 for PE5-0 logic analyzer E
void LogicAnalyzerE(void){  // called 10k/sec
  UART0_DR_R = GPIO_PORTE_DATA_R|0x80;   // send data to TExaSdisplay
}

// ------------PeriodicTask2_Init------------
// Activate an interrupt to run a user task periodically.
// Give it a priority 0 to 6 with lower numbers
// signifying higher priority.  Equal priority is
// handled sequentially.
// Input:  task is a pointer to a user function
//         Bus clock frequency in Hz
//         freq is number of interrupts per second
//           1 Hz to 10 kHz
//         priority is a number 0 to 6
// Output: none
void (*PeriodicTask2)(void);   // user function
void PeriodicTask2_Init(void(*task)(void), 
  uint32_t busfrequency, uint32_t freq, uint8_t priority){//long sr;
    uint32_t volatile delay;
  if((freq == 0) || (freq > 10000)){
    return;                        // invalid input
  }
  if(priority > 6){
    priority = 6;
  }

  PeriodicTask2 = task;            // user function
  // ***************** Timer5 initialization *****************
  SYSCTL_RCGCTIMER_R |= 0x20;      // 0) activate clock for Timer5
 // while((SYSCTL_PRTIMER_R&0x20) == 0){};// allow time for clock to stabilize
    delay = SYSCTL_RCGCTIMER_R;
  TIMER5_CTL_R &= ~TIMER_CTL_TAEN; // 1) disable Timer5A during setup
                                   // 2) configure for 32-bit timer mode
  TIMER5_CFG_R = TIMER_CFG_32_BIT_TIMER;
                                   // 3) configure for periodic mode, default down-count settings
  TIMER5_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
                                   // 4) reload value
  TIMER5_TAILR_R = (busfrequency/freq - 1);
  TIMER5_TAPR_R = 0;               // 5) bus clock resolution
                                   // 6) clear TIMER5A timeout flag
  TIMER5_ICR_R = TIMER_ICR_TATOCINT;
  TIMER5_IMR_R |= TIMER_IMR_TATOIM;// 7) arm timeout interrupt
                                   // 8) priority
  NVIC_PRI23_R = (NVIC_PRI23_R&0xFFFFFF00)|(priority<<5);
// interrupts enabled in the main program after all devices initialized
// vector number 108, interrupt number 92
  NVIC_EN2_R = 1<<28;              // 9) enable IRQ 92 in NVIC
  TIMER5_CTL_R |= TIMER_CTL_TAEN;  // 10) enable Timer5A 32-b

}

void Timer5A_Handler(void){
  TIMER5_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER5A timeout
  (*PeriodicTask2)();               // execute user task
}

// ------------PeriodicTask2_Stop------------
// Deactivate the interrupt running a user task
// periodically.
// Input: none
// Output: none
void PeriodicTask2_Stop(void){
  if(SYSCTL_RCGCTIMER_R&0x20){
    // prevent hardware fault if PeriodicTask2_Init() has not been called
    TIMER5_ICR_R = TIMER_ICR_TATOCINT;// clear TIMER5A timeout flag
    NVIC_DIS2_R = 1<<28;           // disable IRQ 92 in NVIC
  }
}

#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable


//------------UART0_Init------------
// Busy-wait verion, 
// Initialize the UART for 115,200 baud rate (assuming busfrequency system clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
//        bus clock frequency in Hz
//        baud rate in bps
// Output: none
void UART0_Init(void){
  SYSCTL_RCGCUART_R |= 0x01; // activate UART0
  SYSCTL_RCGCGPIO_R |= 0x01; // activate port A
  UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART

  UART0_IBRD_R = 43;                    // IBRD = int(80,000,000 / (16 * 115200)) = int(43.402778)
  UART0_FBRD_R = 26;                    // FBRD = round(0.402778 * 64) = 26
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CTL_R |= UART_CTL_UARTEN;       // enable UART
  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1-0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1-0
                                        // configure PA1-0 as UART
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA
}

//------------UART0_InChar------------
// Busy-wait verion, wait for new serial port input
// Input: none
// Output: ASCII code for key typed
char UART0_InChar(void){
  while((UART0_FR_R&UART_FR_RXFE) != 0);
  return((char)(UART0_DR_R&0xFF));
}

//------------UART0_OutChar------------
// Busy-wait verion, Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART0_OutChar(char data){
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}
//------------UART0_OutString------------
// Busy-wait verion, Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART0_OutString(char *pt){
  while(*pt){
    UART0_OutChar(*pt);
    pt++;
  }
}
//---------------------OutCRLF0---------------------
// Busy-wait verion, Output a CR,LF to UART to go to a new line
// Input: none
// Output: none
void OutCRLF0(void){
  UART0_OutChar(CR);
  UART0_OutChar(LF);
}


// start conversions, sample always
// ADC1
// PD3 Ain4
// 16-point averaging 125kHz sampling
void ADC1_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGCADC_R |= 0x02;       // 1) ADC1 clock
  SYSCTL_RCGCGPIO_R |= 0x08;      // 2) activate clock for Port D
  while((SYSCTL_PRGPIO_R&0x08) == 0){};// allow time for clock to stabilize
  GPIO_PORTD_DIR_R &= ~0x08;      // 3) make PD3 input
  GPIO_PORTD_AFSEL_R |= 0x08;     // 4) enable alternate function on PD3
  GPIO_PORTD_DEN_R &= ~0x08;      // 5) disable digital I/O on PD3
  GPIO_PORTD_AMSEL_R |= 0x08;     // 6) enable analog functionality on PD3
  for(delay = 0; delay<20; delay++){};  // allow time for clock to stabilize
  ADC1_PC_R = 0x01;               // 7) 125K rate
  ADC1_SSPRI_R = 0x0123;          // 8) Sequencer 3 is highest priority
  ADC1_ACTSS_R = 0x0000;          // 9) disable sample sequencer 3
  ADC1_EMUX_R |= 0xF000;          // 10) seq3 is always/continuous trigger
  ADC1_SAC_R = 0x03;              //   8-point average 125kHz/8 = 15,625 Hz
  ADC1_SSMUX3_R = 4;              // 11) set channel 4
  ADC1_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
  ADC1_IM_R = 0x0000;             // 13) disable SS3 interrupts
  ADC1_ACTSS_R = 0x0008;          // 14) enable sample sequencer 3
}
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
extern char EID1[16];  // student's EID
extern char EID2[16];  // student's EID
static int PortMode=0;    // randomly assigns 0 to 1
// Piano key, 0 is PA5-2
//            1 is PE3-0
static int NotesMode=0; // randomly assigns 0 to 7
static int Fail=0;
static uint32_t Time=0;
static uint32_t Score=0;
#define PB543210     (volatile uint32_t *)0x400050FC // bits 5-0
#define PE543210     (volatile uint32_t *)0x400240FC // bits 5-0
#define PA543        (volatile uint32_t *)0x400040E0 // bits 5-3
#define PA5432       (volatile uint32_t *)0x400040F0 // bits 5-2
#define PA432        (volatile uint32_t *)0x40004070 // bits 4-2
#define PC765        (volatile uint32_t *)0x40006380 // bits 7-5
#define PC654        (volatile uint32_t *)0x400061C0 // bits 6-4
#define PE321        (volatile uint32_t *)0x40024038 // bits 3-1
#define PE3210       (volatile uint32_t *)0x4002403C // bits 3-0
#define PE210        (volatile uint32_t *)0x4002401C // bits 2-0
#define PF321        (volatile uint32_t *)0x40025038 // bits 3-1
#define PortADir     (volatile uint32_t *)0x40004400 
#define PortBDir     (volatile uint32_t *)0x40005400 
#define PortCDir     (volatile uint32_t *)0x40006400 
#define PortEDir     (volatile uint32_t *)0x40024400 
#define PortADen     (volatile uint32_t *)0x4000451C 
#define PortBDen     (volatile uint32_t *)0x4000551C 
#define PortCDen     (volatile uint32_t *)0x4000651C 
#define PortEDen     (volatile uint32_t *)0x4002451C 
struct port{
  volatile uint32_t *outPt;
  volatile uint32_t *outDirPt;
  volatile uint32_t *outDenPt;
  volatile uint32_t *inPt;
  volatile uint32_t *inDirPt;
  volatile uint32_t *inDenPt;
  uint32_t inshift;
  uint32_t clockMask; // output, and input
};
typedef const struct port port_t;
port_t ports[2]={
  {PB543210,PortBDir,PortBDen,PA5432,PortADir,PortADen, 2, 0x03},
  {PB543210,PortBDir,PortBDen,PE3210,PortEDir,PortEDen, 0, 0x12}
};
const char PortProblem[2][80]={
  "Key3=PA5, Key2=PA4, Key1=PA3, Key0=PA2, DAC=PB5-0",
  "Key3=PE3, Key2=PE2, Key1=PE1, Key0=PE0, DAC=PB5-0"
};
const char NotesProblem[8][80]={
  "Key0=329.6, Key1=415.3, Key2=493.9, Key3=554.4 Hz",        // NotesMode=0
  "Key0=311.1, Key1=392.0, Key2=466.2, Key3=523.3 Hz",        // NotesMode=1
  "Key0=293.7, Key1=370.0, Key2=440.0, Key3=493.9 Hz",        // NotesMode=2
  "Key0=277.2, Key1=349.2, Key2=415.3, Key3=466.2 Hz",        // NotesMode=3
  "Key0=261.6, Key1=329.6, Key2=392.0, Key3=440.0 Hz",        // NotesMode=4
  "Key0=246.9, Key1=311.1, Key2=370.0, Key3=415.3 Hz",        // NotesMode=5
  "Key0=233.1, Key1=293.7, Key2=349.2, Key3=392.0 Hz",        // NotesMode=6
  "Key0=220.0, Key1=277.2, Key2=329.6, Key3=370.0 Hz"         // NotesMode=7
};
const uint32_t NotesPeriod[8][4]={ // interrupt periods
//{15169, 12039, 10124, 9019},
//{16071, 12755, 10726, 9556},
//{17026, 13514, 11364, 10124},
//{18039, 14317, 12039, 10726},
//{19111, 15169, 12755, 11364},
//{20248, 16071, 13514, 12039},
//{21452, 17026, 14317, 12755},
//{22727, 18039, 15169, 13514}
//};
{30337, 24079, 20248, 18039},
{32141, 25511, 21452, 19111},
{34052, 27027, 22727, 20248},
{36077, 28635, 24079, 21452},
{38223, 30337, 25511, 22727},
{40495, 32141, 27027, 24079},
{42903, 34052, 28635, 25511},
{45455, 36077, 30337, 27027}
};



void OutNotesProblem(void){
  UART0_OutString((char*)NotesProblem[NotesMode]); OutCRLF0();  
}


  
int CheckClock(void){
  uint32_t mask = ports[PortMode].clockMask;
  if( (mask&SYSCTL_RCGCGPIO_R) == mask){
    return 1; // correct
  }
  Fail = 1;
  return 0;
}

void CheckInitialization(void){
  UART_OutString("Initialization, ");
  if(CheckClock()==0){
    UART_OutString("RCGCGPIO is not correct\n\r");
    Fail = 1;
    return;
  }
  uint32_t outdir = *ports[PortMode].outDirPt;
  if((outdir&0x3F)!=0x3F){
    UART_OutString("Direction register for the output is not correct\n\r");
    Fail = 1;
    return;
  }
  uint32_t outden = *ports[PortMode].outDenPt;
  if((outden&0x3F)!=0x3F){
    UART_OutString("Data enable register for the output is not correct\n\r");
    Fail = 1;
    return;
  }
  uint32_t indir = *ports[PortMode].inDirPt;
  indir = (indir >>ports[PortMode].inshift)&0x07;
  if(indir!= 0){
    UART_OutString("Direction register for the input is not correct\n\r");
    Fail = 1;
    return;
  }
// simulator bug on writing PortA DEN
//  uint32_t inden = *ports[PortMode].inDenPt;
//  inden = (inden >>ports[PortMode].inshift)&0x07;
//  if(inden != 0x07){
//    UART_OutString("Data enable register for the input is not correct\n\r");
//    Fail = 1;
//    return;
//  }  

  UART_OutString("good, Score=4\n\r");
  Score = 4;  
  Fail = 0; 
}
uint32_t ReadInput(void){
  if(CheckClock()){
    return (*ports[PortMode].inPt)>>ports[PortMode].inshift;
  }
  UART_OutString("RCGCGPIO is not correct\n\r");
  Fail = 1;
  return 0;
}
uint32_t ReadOutput(void){
  if(CheckClock()){
    uint32_t dac = (*ports[PortMode].outPt);       // bits 5,4,3,2,1,0
    return dac;
  }
  UART_OutString("RCGCGPIO is not correct\n\r");
  Fail = 1;
  return 0;
}




uint32_t M;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}  
uint32_t M6;
uint32_t Random6(void){
  M6 = 1664525*M6+1013904223;
  return M6>>26; // 0 to 63
}  
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}  
char setUpper(char in){
  if((in >= 'a')&&(in <= 'z')){
    return in-0x20;
  }
  return in;
}
//---------------------OutCRLF---------------------
// Output a CR,LF to UART to go to a new line
// Input: none
// Output: none
void OutCRLF(void){
  UART_OutChar(CR);
  UART_OutChar(LF);
}
uint32_t LastIn,State,Mask,SampleFlag;   // previous input state
uint32_t SimulationFlag=0;
uint32_t Last16ADCresults[16];     // average the 64 most recent ADC conversions, filled in ADC1_In()
uint32_t Last16ADCindex = 0;       // index into Last16ADCresults array
// These values are multiplied by the data to calculate the FFT.
// The final result will be 17 times too large, due to integer math.
// sqrt(2)/2 = 0.7071 and 12/17 = 0.7059
const int32_t RMULTIPLIERS[8] = {17, 12, 0, -12, -17, -12, 0, 12};
const int32_t IMULTIPLIERS[8] = {0, -12, -17, -12, 0, 12, 17, 12};


// calculate FFT with frequency bins of 1/(16*newADCperiod), and
// return 0 to 6 points earned in the test.  
uint32_t FFT(void){
  int32_t rbin0=0, rbin1=0, rbin2=0, rbin3=0, rbin4=0, rbin5=0, rbin6=0, rbin7=0;
  int32_t ibin1=0, ibin2=0, ibin3=0, ibin4=0, ibin5=0, ibin6=0, ibin7=0;
  int32_t mag0, mag1, mag2, mag3, mag4, mag5, mag6, mag7;
  int32_t i;
  // restore settings for Timer5 and ADC1 to what they were before this
  // special Background Test began
 
  // calculate the magnitude of the FFT using some shortcuts
  // bin 0: 0 to 1/16*TargetFrequency
  //        will be large due to necessary ~1.65 V offset
  // bin 1: 1/16*TargetFrequency to 2/16*TargetFrequency
  //        should be largest bin by far for sine wave of TargetFrequency
  // bins 2-6: n/16*TargetFrequency to (n+1)/16*TargetFrequency
  //        should all be much smaller than bin 1 for sine wave
  // bin 7: 7/16*TargetFrequency to 8/16*TargetFrequency
  //        complex conjugate equal in magnitude to bin 1
  // assumptions: the 0 to 3 V ADC causes clipping but it doesn't matter
  // make sure that 16 samples have been taken

  for(i=0; i<16; i=i+1){
    // calculate bin 0
    rbin0 = rbin0 + Last16ADCresults[i];
    // calculate bin 1
    rbin1 = rbin1 + Last16ADCresults[i]*RMULTIPLIERS[i&0x7];
    ibin1 = ibin1 + Last16ADCresults[i]*IMULTIPLIERS[i&0x7];
    // calculate bin 2
    rbin2 = rbin2 + Last16ADCresults[i]*RMULTIPLIERS[(2*i)&0x7];
    ibin2 = ibin2 + Last16ADCresults[i]*IMULTIPLIERS[(2*i)&0x7];
    // calculate bin 3
    rbin3 = rbin3 + Last16ADCresults[i]*RMULTIPLIERS[(3*i)&0x7];
    ibin3 = ibin3 + Last16ADCresults[i]*IMULTIPLIERS[(3*i)&0x7];
    // calculate bin 4
    rbin4 = rbin4 + Last16ADCresults[i]*RMULTIPLIERS[(4*i)&0x7];
    ibin4 = ibin4 + Last16ADCresults[i]*IMULTIPLIERS[(4*i)&0x7];
    // calculate bin 5
    rbin5 = rbin5 + Last16ADCresults[i]*RMULTIPLIERS[(5*i)&0x7];
    ibin5 = ibin5 + Last16ADCresults[i]*IMULTIPLIERS[(5*i)&0x7];
    // calculate bin 6
    rbin6 = rbin6 + Last16ADCresults[i]*RMULTIPLIERS[(6*i)&0x7];
    ibin6 = ibin6 + Last16ADCresults[i]*IMULTIPLIERS[(6*i)&0x7];
    // calculate bin 7
    rbin7 = rbin7 + Last16ADCresults[i]*RMULTIPLIERS[(7*i)&0x7];
    ibin7 = ibin7 + Last16ADCresults[i]*IMULTIPLIERS[(7*i)&0x7];
  }
  // calculate the magnitude^2 = (real^2 + imaginary^2)
  // before squaring, divide by 68 to prevent overflow
  mag0 = (rbin0/68)*(rbin0/68);
  mag1 = (rbin1/68)*(rbin1/68) + (ibin1/68)*(ibin1/68);
  mag2 = (rbin2/68)*(rbin2/68) + (ibin2/68)*(ibin2/68);
  mag3 = (rbin3/68)*(rbin3/68) + (ibin3/68)*(ibin3/68);
  mag4 = (rbin4/68)*(rbin4/68) + (ibin4/68)*(ibin4/68);
  mag5 = (rbin5/68)*(rbin5/68) + (ibin5/68)*(ibin5/68);
  mag6 = (rbin6/68)*(rbin6/68) + (ibin6/68)*(ibin6/68);
  mag7 = (rbin7/68)*(rbin7/68) + (ibin7/68)*(ibin7/68);
  // evaluate the magnitude^2 and award points
  // for no input:
  // mag0 <      25,000
  if(mag0 < 25000){
    return 0;
  }
  // mag0 =
  // for perfect sine wave:
  // mag0 =   3,713,329
  // mag1 = 263,513,000
  // mag2 =           8
  // mag3 =     134,765
  // mag4 =           0
  // mag5 =     134,765
  // mag6 =           8
  // mag7 = 263,513,000
  if((mag0 <   1000000) || (mag0 > 20000000) ||
     (mag1 <  50000000) ||
     (mag2 >   1000000) ||
     (mag3 >   1000000) ||
     (mag4 >   1000000) ||
     (mag5 >   1000000) ||
     (mag6 >   1000000) ||
     (mag7 <  50000000)){
    return 0;                           // at least one bad magnitude; award no points
  }
  return 6;
}

void Lab6Grader(void){ uint32_t in;
  if(Fail){
    UART_OutString("Halted, Score= ");
    UART_OutUDec(Score);OutCRLF();
    Timer3A_Stop();
  }
  if(SampleFlag == 1){
    if(SimulationFlag){
      if(Last16ADCindex<16){
        Last16ADCresults[Last16ADCindex] = ReadOutput()<<6;
      }else{
        Last16ADCresults[Last16ADCindex&0x0F] += ReadOutput()<<6;
      }
    }else{
      if(Last16ADCindex<16){
        Last16ADCresults[Last16ADCindex] = ADC1_SSFIFO3_R;
      }else{
        Last16ADCresults[Last16ADCindex&0x0F] += ADC1_SSFIFO3_R;
      }
    }
    Last16ADCindex++;
    if(Last16ADCindex>=64){
      SampleFlag = 2;
      TIMER3_TAILR_R = 800000; // 100 Hz
    }
    return;
  }
  Time++;
  if(Time == 10){
    CheckInitialization();
    if(Fail == 0){
      UART_OutString("Activate piano Key0");
      State = 0; //
      Mask = 1;  // looking for Key0
      SampleFlag = 0; // start looking
    }
  }else if(Time > 10){
    in = ReadInput();
    if(in == LastIn){
      if((in == Mask)&&(SampleFlag ==0)){
        TIMER3_TAILR_R = NotesPeriod[NotesMode][State]; // 16 times expected freq
        SampleFlag = 1; // start looking
        Last16ADCindex = 0;
      }
    }
    if(SampleFlag == 2){
      // do FFT
      uint32_t result = FFT();
      if(result){
        Score = Score+result;
        if(Score>25) Score = 25;
        UART_OutString(" Good. Score=");
        UART_OutUDec(Score); OutCRLF();
      }else{
        UART_OutString(" Waveform wrong frequency or not sinusoidal\n\r");
      }
      State++;
      Mask = Mask<<1;
      if(State < 4){
        SampleFlag = 0;
        UART_OutString("Activate piano Key");
        UART_OutUDec(State);  
      }else{
        UART_OutString("Done. Score=");
        UART_OutUDec(Score); OutCRLF();
        SampleFlag = 3;
        Timer3A_Stop();
      }
    }
    LastIn = in;          
  }
}
// ************TExaS_Init*****************
// Initialize grader, triggered by periodic timer
// Bus clock frequency will be 80 MHz
// This needs to be called once
// Inputs: Scope Logic analyzer or Grader
// Outputs: none
// 0 for TExaS oscilloscope on PD3, 
// 1 for PA7-2 logic analyzer A
// 2 for PB6-0 logic analyzer B
// 3 for PC7-4 logic analyzer C
// 4 for PE5-0 logic analyzer E
// 5 for Lab6 real board grader, 
// 6 for Lab6 simulation grader
// 7 for none
void TExaS_Init(enum TExaSmode mode){int flag;
  DisableInterrupts();
  PLL_Init(Bus80MHz);      // 80  MHz
  UART0_Init();    // busy-wait initialize UART
  M = 4;
  Score=0;
  State = 0;
  Last16ADCindex = 0;
  Mask = 1;
  SampleFlag = 0;
  EnableInterrupts();
  UART0_OutString("Lab 6, Spring 2023, ");
  if((strcmp(EID1,"abc123"))&&strcmp(EID2,"abc123")){
    for(int i=0; EID1[i]; i++){
      M = M*setUpper(EID1[i]);
    }
    for(int i=0; EID2[i]; i++){
      M = M*setUpper(EID2[i]);
    }
    PortMode = (Random32()>>31); // 0,1
    //PortMode = 0; // force for debugging
    NotesMode = (Random32()>>29); // 0,1,...7
    //NotesMode = 7; // force for debugging
    UART0_OutString("EID1="); 
    for(int i=0; EID1[i]; i++){
      UART0_OutChar(setUpper(EID1[i]));
    } 
    UART0_OutString(", EID2="); 
    for(int i=0; EID2[i]; i++){
      UART0_OutChar(setUpper(EID2[i]));
    } 
    OutCRLF0();
    UART0_OutString((char*)PortProblem[PortMode&0x07]); OutCRLF0();  
    UART0_OutString((char*)NotesProblem[NotesMode]); OutCRLF0();  
    flag =1;
  }else{
    flag = 0;
    UART0_OutString("\n\rPlease enter your two EIDs into Lab 6.c\n\r");
  }  

  if(mode == LOGICANALYZERA){
  // enable 10k periodic interrupt if logic analyzer mode
   SYSCTL_RCGCGPIO_R |= 0x01; // port A needs to be on
   PeriodicTask2_Init(&LogicAnalyzerA,80000000,10000,5); // run logic analyzer
   EnableInterrupts();
  }
  if(mode == LOGICANALYZERB){
  // enable 10k periodic interrupt if logic analyzer mode
   SYSCTL_RCGCGPIO_R |= 0x02; // port B needs to be on
   PeriodicTask2_Init(&LogicAnalyzerB,80000000,10000,5); // run logic analyzer
   EnableInterrupts();
  }
  if(mode == LOGICANALYZERC){
  // enable 10k periodic interrupt if logic analyzer mode
   SYSCTL_RCGCGPIO_R |= 0x04; // port C needs to be on
   PeriodicTask2_Init(&LogicAnalyzerC,80000000,10000,5); // run logic analyzer
   EnableInterrupts();
  }
  if(mode == LOGICANALYZERE){
  // enable 10k periodic interrupt if logic analyzer mode
   SYSCTL_RCGCGPIO_R |= 0x10; // port E needs to be on
   PeriodicTask2_Init(&LogicAnalyzerE,80000000,10000,5); // run logic analyzer
   EnableInterrupts();
  }

  if(mode == SCOPE){
    ADC1_Init();  // activate PD3 as analog input
    PeriodicTask2_Init(&Scope,80000000,10000,5); // run scope at 10k
    EnableInterrupts();
  }
  if((mode == SIMULATIONGRADER)&&flag){
    UART_Init(); // interrupting version
    SimulationFlag=1;
    EnableInterrupts();
    //needed interrupt version of UART

    Timer3A_Init(&Lab6Grader, 800000,1); // 100Hz
    Time = 0;
  }
  if((mode == REALBOARDGRADER)&&flag){
    UART_Init(); // interrupting version
    SimulationFlag=0;
    ADC1_Init();  // activate PD3 as analog input
    EnableInterrupts();
    //needed interrupt version of UART
    UART_OutString("\n\rConnect PD3 to DACOUT.\n\r");

    Timer3A_Init(&Lab6Grader, 800000,1); // 100Hz
    Time = 0;
  }
}

// ************TExaS_Stop*****************
// Stop the transfer
// Inputs:  none
// Outputs: none
void TExaS_Stop(void){
  PeriodicTask2_Stop();
}


