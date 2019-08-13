// PLLTestMain.c
// Runs on LM4F120/TM4C123
// Test the PLL function to verify that the system clock is
// running at the expected rate.  Use the debugger if possible
// or an oscilloscope connected to PF2.
// The #define statement in the file PLL.h allows PLL_Init() to
// initialize the PLL to the desired frequency.  When using an
// oscilloscope to look at LED1, it should be clear to see that
// the LED flashes about 2 (80/40) times faster with a 80 MHz 
// clock than with a 40 MHz clock.
// Daniel Valvano
// September 11, 2013

#include <stdint.h>
#include "C:\Keil\Labware\inc\tm4c123gh6pm.h"

#define GPIO_PORTF2     (*((volatile uint32_t *)0x40025010))
#define GPIO_PORTF4     (*((volatile uint32_t *)0x40025040))
#define SYSCTL_RIS_PLLLRIS      0x00000040  // PLL Lock Raw Interrupt Status
#define SYSCTL_RCC_XTAL_M       0x000007C0  // Crystal Value
#define SYSCTL_RCC_XTAL_6MHZ    0x000002C0  // 6 MHz Crystal
#define SYSCTL_RCC_XTAL_8MHZ    0x00000380  // 8 MHz Crystal
#define SYSCTL_RCC_XTAL_16MHZ   0x00000540  // 16 MHz Crystal
#define SYSCTL_RCC2_USERCC2     0x80000000  // Use RCC2
#define SYSCTL_RCC2_DIV400      0x40000000  // Divide PLL as 400 MHz vs. 200
                                            // MHz
#define SYSCTL_RCC2_SYSDIV2_M   0x1F800000  // System Clock Divisor 2
#define SYSCTL_RCC2_SYSDIV2LSB  0x00400000  // Additional LSB for SYSDIV2
#define SYSCTL_RCC2_PWRDN2      0x00002000  // Power-Down PLL 2
#define SYSCTL_RCC2_BYPASS2     0x00000800  // PLL Bypass 2
#define SYSCTL_RCC2_OSCSRC2_M   0x00000070  // Oscillator Source 2
#define SYSCTL_RCC2_OSCSRC2_MO  0x00000000  // MOSC
// the PLL to the desired frequency.

#define SYSDIV2 19

//#define SYSDIV2 7

// delay function for testing from sysctl.c
// which delays 3*ulCount cycles

//Keil uVision Code
// ignore keil error saying expected '(' after 'asm'
// subs: subtract 1 from r0
// bne: branch if not equal
// bx branch and exchange instruction set
// r14(lr) link register: store the return location for functions
	__asm void
	Delay(unsigned long ulCount)
	{
    subs    r0, #1
    bne     Delay
    bx      lr
	}
	
	unsigned long Time[50];
	unsigned long Data[100];
	int i = 0;
	int j = 0;

void PortF_init(){
	SYSCTL_RCGC2_R |= 0x00000020; // 1) Enable PORTF clock bit 5
	while((SYSCTL_PRGPIO_R&0x0020) == 0){};// ready? bit 5 of PRGPIO should be set
	GPIO_PORTF_LOCK_R = 0x4C4F434B; // 2) unlock PortF PF0
	GPIO_PORTF_CR_R |= 0x1F; // allow changes to PF4-0, set bits 0-5
	GPIO_PORTF_AMSEL_R &= 0x00; // 3) disable analog function, clear all bits
	GPIO_PORTF_PCTL_R &= 0x00000000; // 4) GPIO clear bit PCTL
	GPIO_PORTF_DIR_R &= ~0x11; // 5.1) PF4,PF0 input, clear bits 0,5
	GPIO_PORTF_DIR_R |= 0x08; // 5.2) PF3 output , set bit 4
	GPIO_PORTF_AFSEL_R &= ~0x1F; // 6) no alternate functions, clear bits0-5
	GPIO_PORTF_PUR_R |= 0x11; // enable pullup resistors on PF4,PF0
	// set bits 0,5
	GPIO_PORTF_DEN_R |= 0x1F; // 7) enable digital pins PF4-PF0 , set bits0-5
}

void PortD_init(){
  SYSCTL_RCGCGPIO_R |= 0x08;   // activate port D
  while((SYSCTL_PRGPIO_R&0x0008) == 0){};// ready? bit 5 of PRGPIO should be set
  //PRGPIO register indicates whether the GPIO modules are ready to be accessed by software (datasheet page 406)
	GPIO_PORTD_LOCK_R = 0x4C4F434B; // unlock register
	GPIO_PORTD_CR_R |= 0x01;        // allow changes to PD3-0
  GPIO_PORTD_DIR_R |= 0x01;       // make PD3 -> PD0 output
  //GPIO_PORTD_AFSEL_R &= ~0x04; // regular port function
  GPIO_PORTD_DEN_R |= 0x01;       // enable digital I/O on PD3-0
  GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFF0FFFF)+0x00000000;
  GPIO_PORTD_AMSEL_R = 0;         // disable analog functionality on PD
}

void PLL_init(){
// Your code goes here!
	
// 1) bypass PLL while initializing

SYSCTL_RCC2_R |= 0x80000800; // BYPASS2, PLL bypass

// 2) select the crystal value and oscillator source

SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0) + 0x00000540; // clear bits 10-6 ... 10101, configure for 16 MHz crystal

SYSCTL_RCC2_R &= ~0x00000070; // configure for main oscillator source

// 3) activate PLL by clearing PWRDN

SYSCTL_RCC2_R &= ~0x00002000;

// 4) set the desired system divider

SYSCTL_RCC2_R |= 0x40000000; // use 400 MHz PLL

SYSCTL_RCC2_R = (SYSCTL_RCC2_R& ~0x1FC00000) + ( SYSDIV2<<22); // 20 MHz

// 5) wait for the PLL to lock by polling PLLLRIS

while(( SYSCTL_RIS_R & SYSCTL_RIS_PLLLRIS) == 0){}; // wait for PLLRIS bit

// 6) enable use of PLL by clearing BYPASS

SYSCTL_RCC2_R &= ~0x00000800; 
}

void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = 0x00FFFFFF;
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 0x00000005;
}

void SysTick_Wait(uint32_t delay){
	NVIC_ST_RELOAD_R = delay - 1;
	NVIC_ST_CURRENT_R = 0;
	while((NVIC_ST_CTRL_R & 0x00010000) == 0){}
}

void SysTick_Wait10ms(uint32_t delay){
	uint32_t i;
	for(i = 0; i < delay; i++){
		SysTick_Wait(800000); // wait 10 ms
	}
}

#define delay 10000000 // .5 seconds
//#define delay 800000 // 10 ms

void LetterS(void){
	//The Letter S using Morse code
	unsigned long last = NVIC_ST_CURRENT_R;
	GPIO_PORTD_DATA_R |= 0x01; SysTick_Wait(delay); // .50 seconds
	if(i < 50){
		Data[i] = GPIO_PORTD_DATA_R & 0x01;
	}
	//Data[i] = GPIO_PORTD_DATA_R & 0x01;
	unsigned long now = NVIC_ST_CURRENT_R;
	if(i < 50){
		Time[i] = (last-now) & 0x00FFFFFF; // 24-bit time difference
	}
	GPIO_PORTD_DATA_R &= ~0x01; Data[i+1] = GPIO_PORTD_DATA_R & 0x01; SysTick_Wait(delay/2); // wait .25 seconds
	GPIO_PORTD_DATA_R |= 0x01; SysTick_Wait(delay);
	GPIO_PORTD_DATA_R &= ~0x01; SysTick_Wait(delay/2);
	GPIO_PORTD_DATA_R |= 0x01; SysTick_Wait(delay);
	GPIO_PORTD_DATA_R &= ~0x01; SysTick_Wait(delay);
}

void LetterO(void){
	//The Letter O using Morse code
	GPIO_PORTD_DATA_R |= 0x01; SysTick_Wait(delay); SysTick_Wait(delay); // 1 seconds
	GPIO_PORTD_DATA_R &= ~0x01; SysTick_Wait(delay/2);
	GPIO_PORTD_DATA_R |= 0x01; SysTick_Wait(delay); SysTick_Wait(delay);
	GPIO_PORTD_DATA_R &= ~0x01; SysTick_Wait(delay/2);
	GPIO_PORTD_DATA_R |= 0x01; SysTick_Wait(delay); SysTick_Wait(delay);
	GPIO_PORTD_DATA_R &= ~0x01;SysTick_Wait(delay);
}

void FlashSOS(void){
	//S
	LetterS();
	//O
	LetterO();
	//S
	LetterS();
	for(int i = 0; i < 6; i++){ // Delay for 3 seconds inbetween flashes
		SysTick_Wait(delay);
	}
}

unsigned long SW1; // input from PF4
unsigned long SW2; // input from PF0
int main(void){  
	
	PLL_init();
	SysTick_Init();
	PortF_init();
	PortD_init();
	
	while(1){
		do{
			SW1 = GPIO_PORTF_DATA_R&0x10; // PF4 into SW1
		}while(SW1 == 0x10);
		do{
			FlashSOS();
			SW2 = GPIO_PORTF_DATA_R&0x01; // PF0 into SW2
			i++;
		}while(SW2 == 0x01);
	}
}
