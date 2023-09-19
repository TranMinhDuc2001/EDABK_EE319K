;****************** main.s ***************
; Program initially written by: Yerraballi and Valvano
; Author: Place your name here
; Date Created: 1/15/2018 
; Last Modified: 12/30/2022 
; Brief description of the program: Solution to Lab1
; The objective of this system is to implement a parity system
; Hardware connections: 
;  One output is positive logic, 1 turns on the LED, 0 turns off the LED
;  Three inputs are positive logic, meaning switch not pressed is 0, pressed is 1
GPIO_PORTD_DATA_R  EQU 0x400073FC
GPIO_PORTD_DIR_R   EQU 0x40007400
GPIO_PORTD_DEN_R   EQU 0x4000751C
GPIO_PORTE_DATA_R  EQU 0x400243FC
GPIO_PORTE_DIR_R   EQU 0x40024400
GPIO_PORTE_DEN_R   EQU 0x4002451C
SYSCTL_RCGCGPIO_R  EQU 0x400FE608
       PRESERVE8 
       AREA   Data, ALIGN=2
; Declare global variables here if needed
; with the SPACE assembly directive
       ALIGN 4
       AREA    |.text|, CODE, READONLY, ALIGN=2
       THUMB
       EXPORT EID
EID    DCB "20203370",0  ;replace ABC123 with your EID
       EXPORT RunGrader
	   ALIGN 4
RunGrader DCD 1 ; change to nonzero when ready for grading
           
      EXPORT  Lab1
;DOC KI HUONG DAN LAB1
;Tham khao cac muc 1.7.3 ebook de su dung MOV: max 16-bit va LDR: max 32-bit , STRB
;Tham khao cac muc 1.7.3 ebook su dung EOR
Lab1
    ; Khoi tao 
    LDR R0, =SYSCTL_RCGCGPIO_R   ; Nap dia chi tu bo nho vao thanh ghi R0
    MOV R1, #0x10                ; luu vao R1 gia tri la 0x10 la bat clock portE
    STR R1, [R0]                 ; luu giá tri cua R1 vào R0 

    LDR R0, =GPIO_PORTE_DIR_R    ; Nap dia chi thanh ghi huong i/o vao thanh ghi R0 
    MOV R1, #0x20                ; lua chon cac chan PE0,1,2 là input chân PE5 là output 
    STR R1, [R0]                 ; luu gia tri cua R1 vao R0
	
	
    LDR R0, =GPIO_PORTE_DEN_R    ; Nap dia chi <thanh ghi du lieu> vao thanh ghi R0
	MOV R1, #0x27              	 ; lua chon cac chan PD0,1,2,5 la cac chan truyen nhan du lieu 
	STR R1, [R0]               	 ; luu cac gia cua R1 vao R0

	LDR R0, =GPIO_PORTE_DATA_R   ; Nap dia chi GPIO_PORTD_DATA_R vao R0
loop
	;Bat dau vong lap
	LDR R1, [R0]
	AND R1,R1,#0x01				  ;Dich phai 0 bit de luu vao PE0
	
	LDR R2, [R0]
	AND R2,R2,#0x02
	LSR R2,R2,#0x01        		  ;Dich phai 1 bit de luu vao PE1
	
	LDR R3, [R0]
	AND R3,R3,#0x04
	LSR R3,R3,#0x02				  ;Dich phai 2 bit de luu vao PE2
		
	;Thuc hien XOR
	EOR R4, R1, R2  			 
	EOR R4, R4, R3  			 

	;Thuc hien luu vao PE5
	LSL R4,R4,#0x05				  ;Dich trai 5 bit de luu vao PE5	
	STR R4,[R0]
	
    B loop                       ; lap lai vong lap

    ALIGN                        ; Dam bao cuoi phan nay duoc can chinh
    END                          ; Ket thuc
               