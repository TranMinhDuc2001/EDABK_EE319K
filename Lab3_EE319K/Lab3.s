;****************** Lab3.s ***************
; Program written by: Put your names here
; Date Created: 2/4/2017
; Last Modified: 1/4/2023
; Brief description of the program
;   The LED toggles at 2 Hz and a varying duty-cycle
; Hardware connections (External: Two buttons and one LED)
;  Change is Button input  (1 means pressed, 0 means not pressed)
;  Breathe is Button input  (1 means pressed, 0 means not pressed)
;  LED is an output (1 activates external LED)
; Overall functionality of this system is to operate like this
;   1) Make LED an output and make Change and Breathe inputs.
;   2) The system starts with the the LED toggling at 2Hz,
;      which is 2 times per second with a duty-cycle of 30%.
;      Therefore, the LED is ON for 150ms and off for 350 ms.
;   3) When the Change button is pressed-and-released increase
;      the duty cycle by 20% (modulo 100%). Therefore for each
;      press-and-release the duty cycle changes from 30% to 70% to 70%
;      to 90% to 10% to 30% so on
;   4) Implement a "breathing LED" when Breathe Switch is pressed:
; PortE device registers
GPIO_PORTE_DATA_R  EQU 0x400243FC
GPIO_PORTE_DIR_R   EQU 0x40024400
GPIO_PORTE_DEN_R   EQU 0x4002451C
SYSCTL_RCGCGPIO_R  EQU 0x400FE608

        IMPORT  TExaS_Init
        THUMB
        AREA    DATA, ALIGN=2
;global variables go here

       AREA    |.text|, CODE, READONLY, ALIGN=3
       THUMB
       EXPORT EID1
EID1   DCB "20203370",0  ;replace ABC123 with your EID
       EXPORT EID2
EID2   DCB "20203370",0  ;replace ABC123 with your EID
       ALIGN 4

     EXPORT  Start


Start
; TExaS_Init sets bus clock at 80 MHz, interrupts, ADC1, TIMER3, TIMER5, and UART0
	MOV R0,#2  ;0 for TExaS oscilloscope, 1 for PORTE logic analyzer, 2 for Lab3 grader, 3 for none
	BL  TExaS_Init ;enables interrupts, prints the pin selections based on EID1 EID2
 ; Your Initialization goes here
 
	LDR R0, =SYSCTL_RCGCGPIO_R	;nap dia chi xung vao R0
	MOV R1, #0x10				;bat xung clock portE
	STR R1,[R0]					;luu gia tri R0 vao R1
	
	LDR R0, =GPIO_PORTE_DIR_R	;nap dia chi thanh ghi dinh huong I/O vao R0
	MOV R1, #0x10					;PE0,PE2 input, PE4 output
	STR R1,[R0]
	
	LDR R0, =GPIO_PORTE_DEN_R	;nap dia chi thanh ghi du lieu vao R0
	MOV R1, #0x15				;PE0,PE2,PE4 la cac chan du lieu 
	STR R1,[R0]
	
	LDR R0, =GPIO_PORTE_DATA_R	;nap dia chi thanh ghi data gpio vao R0

	MOV R7,#10
	MOV R8,#1
	MOV R11,#10
loop  ; main engine goes here
	LDR R2,[R0]
	AND R2,R2,#0x01
	
	LDR R9,[R0]
	AND R9,R9,#0x04
	
	CMP R2,#0x01
	BNE Togle_Led
	BEQ Wait_TL
	
	CMP R9,#0x04
	BEQ Breath_Led
	
	B    loop
	
Breath_Led ;
	PUSH{R3,R4,R5,R6}
	MOV R6,#1000
	LDR R3,[R0]
	ORR R3,R3,#0x10 ; ghi R3 o muc cao -> den sang
	STR R3,[R0]
	MOV R5,R7    ;R7 = 10,30,50,70,90
	MUL R4,R5,R6 ; tao ra R4 de dua vao ham delay
	BL Delay
	BIC R3,R3,#0x10 ; ghi R3 o muc thap -> den tat
	STR R3,[R0]
	RSB R5,R5,#100 ;R5 = 100 - R5
	MUL R4,R5,R6
	BL Delay
	POP{R6,R5,R4,R3}
	ADD R10,R10,#1
	CMP R10,#10 ; cho xung chay 10 lay neu vuot qua 10 thi sang xung khac
	BEQ Func_BL
	B loop
	
Func_BL	; tinh toan R7 = |x| voi x = -90,-70,...,90
	MOV R10,#0
	ADD R11,R11,#20
	CMP R11,#0
	BLT abs
	MOV R7,R11
	CMP R11,#90 ; neu x = 90 thi set lai x = -90
	BEQ Init_BL

	
	BL Breath_Led

abs
	PUSH{R6}
	MOV R6,R11
	NEG R6,R6
	MOV R7,R6
	POP{R6}
	B Breath_Led

Init_BL
	MOV R11,#0xFFFFFFA6 ; xet lai R11 = -90
	B loop

Togle_Led
	PUSH{R3,R4,R5,R6}
	MOV R6,#50000
	LDR R3,[R0]
	ORR R3,R3,#0x10
	STR R3,[R0]
	MOV R5,R7    ;R7 = 10,30,50,70,90
	MUL R4,R5,R6
	BL Delay
	BIC R3,R3,#0x10
	STR R3,[R0]
	RSB R5,R5,#100 ;R5 = 100 - R5
	MUL R4,R5,R6
	BL Delay
	POP{R6,R5,R4,R3}
	MOV R8,#1
	BL loop

Wait_TL
	CMP R8,#1 ; tang bien R7 1 lan trong ca qua trinh wait
	BEQ Increase_TL
	PUSH{R3}
	LDR R3,[R0]
	ORR R3,R3,#0x10
	STR R3,[R0]
	POP{R3}
	
	CMP R9,#0x04
	BEQ Breath_Led
	
	B loop

Increase_TL
	MOV R8,#0
	ADD R7,R7,#20
	CMP R7,#100
	BGT Init ; khi R7 = 90 thi set lai R7 = 10
	BL Wait_TL
	
Init
	MOV R7,#10
	B Wait_TL


 ALIGN 8
Delay ; wait 100ns*R0
	NOP
	NOP
	NOP
	NOP
	SUBS R4,R4,#0x01
	BNE Delay
	BX LR	
	
; 256 points with values from 100 to 9900      
PerlinTable
     DCW 5880,6225,5345,3584,3545,674,5115,598,7795,3737,3775,2129,7527,9020,368,8713,5459,1478,4043,1248,2741,5536,406
     DCW 3890,1516,9288,904,483,980,7373,330,5766,9555,4694,9058,2971,100,1095,7641,2473,3698,9747,8484,7871,4579,1440
     DCW 521,1325,2282,6876,1363,3469,9173,5804,2244,3430,6761,866,4885,5306,6646,6531,2703,6799,2933,6416,2818,5230,5421
     DCW 1938,1134,6455,3048,5689,6148,8943,3277,4349,8866,4770,2397,8177,5191,8905,8522,4120,3622,1670,2205,1861,9479
     DCW 1631,9441,4005,5574,2167,2588,1057,2512,6263,138,8369,3163,2895,8101,3009,5153,7259,8063,3507,789,6570,7756,7603
     DCW 5268,5077,4541,7297,6187,3392,6378,3928,4273,7680,6723,7220,215,2550,2091,8407,8752,9670,4847,4809,291,7833,1555
     DCW 5727,4617,4923,9862,3239,3354,8216,8024,7986,2359,8790,1899,713,2320,751,7067,7335,1172,1708,8637,7105,6608,8254
     DCW 4655,9594,5919,177,1784,5995,6340,2780,8560,5957,3966,6034,6493,1746,6684,445,5038,942,1593,9785,827,3852,4234
     DCW 4311,3124,4426,8675,8981,6914,7182,4388,4081,8445,9517,3813,8828,9709,1402,9364,7488,9211,8139,5613,559,7412
     DCW 6952,6302,9326,3201,2052,5651,9096,9632,636,9249,4196,1976,7450,8292,1287,7029,7718,4158,6110,7144,3316,7909
     DCW 6838,4502,4732,2014,1823,4962,253,5842,9823,5383,9134,7948,3660,8598,4464,2665,1210,1019,2856,9402,5498,5000
     DCW 7565,3086,2627,8330,2435,6072,6991
; 100 numbers from 0 to 10000
; sinusoidal shape


  ALIGN 4
SinTable 
  DCW  5000, 5308, 5614, 5918, 6219, 6514, 6804, 7086, 7361, 7626
  DCW  7880, 8123, 8354, 8572, 8776, 8964, 9137, 9294, 9434, 9556
  DCW  9660, 9746, 9813, 9861, 9890, 9900, 9890, 9861, 9813, 9746
  DCW  9660, 9556, 9434, 9294, 9137, 8964, 8776, 8572, 8354, 8123
  DCW  7880, 7626, 7361, 7086, 6804, 6514, 6219, 5918, 5614, 5308
  DCW  5000, 4692, 4386, 4082, 3781, 3486, 3196, 2914, 2639, 2374
  DCW  2120, 1877, 1646, 1428, 1224, 1036,  863,  706,  566,  444
  DCW   340,  254,  187,  139,  110,  100,  110,  139,  187,  254
  DCW   340,  444,  566,  706,  863, 1036, 1224, 1428, 1646, 1877
  DCW  2120, 2374, 2639, 2914, 3196, 3486, 3781, 4082, 4386, 4692  
      
     ALIGN      ; make sure the end of this section is aligned
     END        ; end of file

