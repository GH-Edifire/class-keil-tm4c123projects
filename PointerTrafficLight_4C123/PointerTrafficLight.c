// PointerTrafficLight.c
// Runs on LM4F120/TM4C123
// Use a pointer implementation of a Moore finite state machine to operate
// a traffic light.
// Daniel Valvano
// September 11, 2013

// east facing red light connected to PB5
// east facing yellow light connected to PB4
// east facing green light connected to PB3
// north facing red light connected to PB2
// north facing yellow light connected to PB1
// north facing green light connected to PB0
// north facing car detector connected to PE1 (1=car present)
// east facing car detector connected to PE0 (1=car present)

#include <stdint.h>
#include "PLL.h"
#include "SysTick.h"
//#include "inc/tm4c123gh6pm.h"
#include "C:\Keil\Labware\inc\tm4c123gh6pm.h"

//#define SYSDIV2 7
#define LIGHT                   (*((volatile uint32_t *)0x400050FC))
#define GPIO_PORTB_OUT          (*((volatile uint32_t *)0x400050FC)) // bits 5-0
#define GPIO_PORTF_IN           (*((volatile uint32_t *)0x40025044)) // bits 4 and 0
#define SENSOR                  (*((volatile uint32_t *)0x40025044))

struct State {
  uint32_t Out;            // 6-bit output
  uint32_t Time;           // 10 ms
  const struct State *Next[4];};// depends on 2-bit input
typedef const struct State STyp;
#define goN   &FSM[0]
#define waitN &FSM[1]
#define goE   &FSM[2]
#define waitE &FSM[3]
STyp FSM[4]={
 {0x21,300,{goN,waitN,goN,waitN}},
 {0x22, 50,{goE,goE,goE,goE}},
 {0x0C,300,{goE,goE,waitE,waitE}},
 {0x14, 50,{goN,goN,goN,goN}}};

 void PortF_Init(void){
	SYSCTL_RCGC2_R |= 0x00000020; // 1) Enable PORTF clock bit 5
	while((SYSCTL_PRGPIO_R&0x0020) == 0){};// ready? bit 5 of PRGPIO should be set
	GPIO_PORTF_LOCK_R = 0x4C4F434B; // 2) unlock PortF PF0
	GPIO_PORTF_CR_R |= 0x11; // allow changes to PF4 and 0
	GPIO_PORTF_AMSEL_R &= 0x00; // 3) disable analog function, clear all bits
	GPIO_PORTF_PCTL_R &= 0x00000000; // 4) GPIO clear bit PCTL
	GPIO_PORTF_DIR_R &= ~0x11; // 5.1) PF4,PF0 input
	//GPIO_PORTF_DIR_R |= 0x08; // 5.2) PF3 output , set bit 4
	GPIO_PORTF_AFSEL_R &= ~0x1F; // 6) no alternate functions, clear bits0-5
	GPIO_PORTF_PUR_R |= 0x11; // enable pullup resistors on PF4,PF0
	GPIO_PORTF_DEN_R |= 0x11; // 7) enable digital pins PF4 and 0
}
 
void PortB_Init(void){
  SYSCTL_RCGC2_R |= 0x00000002;     // Activate clock for Port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};// ready? bit 1 of PRGPIO should be set
  GPIO_PORTB_AMSEL_R = 0x00;        // Disable analog 
  GPIO_PORTB_PCTL_R = 0x00000000;   // PCTL GPIO 
  //GPIO_PORTB_DIR_R &= ~0x03;        //   PB input
	GPIO_PORTB_DIR_R |= 0x3F;         //   PB5-0 output
  GPIO_PORTB_AFSEL_R &= ~0x3F;      // Disable alt funct on PB5-0
  //GPIO_PORTB_PUR_R |= 0x10;         // No pull up
  GPIO_PORTB_DEN_R |= 0x3F;         // Enable digital I/O on PB5-0
}
 
 // Port F switches for inputs, Port B pins 5-0 for outputs
uint32_t Input;
int main(void){
  STyp *Pt;  // state pointer
  PLL_Init();                  // configure for 50 MHz clock
  SysTick_Init();              // initialize SysTick timer
	PortF_Init();
	PortB_Init();
  Pt = goN;                    // initial state: Green north; Red east
  while(1){
		// your code goes here!
		LIGHT = Pt->Out; // set lights
		SysTick_Wait10ms(Pt->Time);
		//Input = ~SENSOR&0x11; // read sensors
		
		// Going North = SW1
		// Going East = SW2
		// switches use positive logic
		// we need to translate bits 4 and 0 to our states
		switch(~SENSOR&0x11){
			case 0x11:
			{
				Input = 3;
				break;
			}
			case 0x10:
			{
				Input = 2;
				break;
			}
			case 0x01:
			{
				Input = 1;
				break;
			}
			case 0x00:
			{
				Input = 0;
				break;
			}
		}
		Pt = Pt->Next[Input]; 
  }
}
