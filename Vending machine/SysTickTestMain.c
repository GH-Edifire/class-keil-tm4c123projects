// SysTickTestMain.c
// Runs on LM4F120/TM4C123
// Test the SysTick functions by activating the PLL, initializing the
// SysTick timer, and flashing an LED at a constant rate.
// Daniel Valvano
// September 12, 2013


#include <stdint.h>
//#include "inc/tm4c123gh6pm.h"
#include "C:\Keil\Labware\inc\tm4c123gh6pm.h"

#include "SysTick.h"
#include "PLL.h"

// PLL = 50 MHz
#define T10ms 500000
#define T20ms 1000000
#define COINS (*((volatile unsigned long *) 0x40025044)) // switches SW1 and SW2, PF4,PF0
#define SODA (*((volatile unsigned long *) 0x40025010)) // Blue LED PF2
#define CHANGE (*((volatile unsigned long *) 0x40025008)) // RED LED PF1
	
#define M0 0
#define W5 1
#define M5 2
#define W10 3
#define M10 4
#define W15 5
#define M15 6
#define W20 7
#define M20 8

unsigned long S; // index into current state
unsigned long Input;

struct State { 
	void (*CmdPt)(void); // output function 
	unsigned long Time; // wait time, 12.5ns units
	unsigned long Next[3];
};
typedef const struct State StateType; 

// Only use Port F, pins 4, 2, 1, 0
void FSM_Init(void){ 
	SYSCTL_RCGC2_R |= 0x00000020; // 1) Enable PORTF clock bit 5
	while((SYSCTL_PRGPIO_R&0x0020) == 0){};// ready? bit 5 of PRGPIO should be set
	GPIO_PORTF_LOCK_R = 0x4C4F434B; // 2) unlock PortF PF0
	GPIO_PORTF_CR_R |= 0x17; // allow changes to PF4, 2, 1, 0
	GPIO_PORTF_AMSEL_R &= 0x00; // 3) disable analog function, clear all bits
	GPIO_PORTF_PCTL_R &= 0x00000000; // 4) GPIO clear bit PCTL
	GPIO_PORTF_DIR_R &= ~0x11; // 5.1) PF4,PF0 input
	GPIO_PORTF_DIR_R |= 0x06; // 5.2) PF2,PF1 output , set bit 3 and bit 2
	GPIO_PORTF_AFSEL_R &= ~0x1F; // 6) no alternate functions, clear bits0-5
	GPIO_PORTF_PUR_R |= 0x11; // enable pullup resistors on PF4,PF0
	GPIO_PORTF_DEN_R |= 0x17; // 7) enable digital pins PF4, PF2, PF1, PF0
	SODA = 0; CHANGE = 0;
}

unsigned long Coin_Input(void){
	unsigned long temp = 0;
	// Get input from switches, translated from negative logic to positive logic
	switch(~COINS&0x11){
		case 0x11: // attempting to put 2 coins at once, default to no input
			{
				temp = 0;
				break;
			}
			case 0x10: // attempt to put in dime, SW1
			{
				temp = 2;
				break;
			}
			case 0x01: // attempt to put in nickel, SW2
			{
				temp = 1;
				break;
			}
			case 0x00: // no input
			{
				temp = 0;
				break;
			}
	}
	return temp; // PF4,0 can be 0, 1, or 2
}

void Solenoid_None(void){ 
};

void Solenoid_Soda(void){ 
	SODA = 0x04; // activate solenoid, PF2
	SysTick_Wait(T10ms); // 10 msec, dispenses a delicious soda
	SODA = 0x00; // deactivate
}

void Solenoid_Change(void){
	CHANGE = 0x02; // activate solenoid, PF1
	SysTick_Wait(T10ms); // 10 msec, return 5 cents
	CHANGE = 0x00;  // deactivate
}

StateType FSM[9]={
{&Solenoid_None, T20ms,{M0,W5,W10}},       // M0, no money
{&Solenoid_None, T20ms,{M5,W5,W5}},         // W5, seeing a nickel
{&Solenoid_None, T20ms,{M5,W10,W15}},     // M5, have 5 cents
{&Solenoid_None, T20ms,{M10,W10,W10}},   // W10, seeing a dime
{&Solenoid_None, T20ms,{M10,W15,W20}},   // M10, have 10 cents
{&Solenoid_None, T20ms,{M15,W15,W15}},   // W15, seeing something
{&Solenoid_Soda, T20ms,{M0,M0,M0}},         // M15, have 15 cents
{&Solenoid_None, T20ms,{M20,W20,W20}},   // W20, seeing dime
{&Solenoid_Change,T20ms,{M15,M15,M15}} // M20, have 20 cents
};


int main(void){
  PLL_Init();                 // set system clock to 50 MHz
  SysTick_Init();           // initialize SysTick timer
// your code!
	FSM_Init();
	S = M0; // Initial State
  while(1){
		// your code!
		// Reminder: SW1 == Dime, SW2 == Nickel
		FSM[S].CmdPt();                // run function first
		SysTick_Wait(FSM->Time);  // default state wait
		Input = Coin_Input();           // check for input
		S = FSM[S].Next[Input];      // switch to next state depending on input
  }
}
