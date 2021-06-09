//Author: Liam Cauquil && Tahsin 

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)


#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "SysTick.h"
#include "TExaS.h"

// Declare your FSM linked structure here
struct State {
  uint32_t OutF;
	uint32_t Out; 
  uint32_t Time;  // 10 ms units
  struct State *Next[16];
}; 
typedef struct State STyp;

#define goN   &FSM[0]
#define waitN &FSM[1]
#define goE   &FSM[2]
#define waitE &FSM[3]
#define wwaitN &FSM[4]
#define wwaitE &FSM[5]
#define walkE &FSM[6]
#define walkN &FSM[7]
#define warningE &FSM[8]
#define warningN &FSM[9]
#define SENSOR (*((volatile uint32_t *)0x4002401C))
#define LIGHT (*((volatile uint32_t *)0x400050FC))
#define WLIGHT (*((volatile uint32_t *)0x40025028))
#define GPIO_PORTB_OUT          (*((volatile uint32_t *)0x400050FC)) // bits 5-0
#define GPIO_PORTE_IN           (*((volatile uint32_t *)0x4002401C)) // bits 2-0
void EnableInterrupts(void);

//FSM Engine
STyp *Pt;
uint32_t Input;

STyp FSM[10] = { //000,				001,  		010,			011  ,		100   ,			101   ,			110   ,			111
 {0x2, 0x21,3,		{goN,				waitN,		goN,			waitN,		wwaitN,			wwaitN,			wwaitN,			wwaitN}}, 	//goN 
 {0x2, 0x22, 2,		{goE,				goE,			goE,			goE, 			walkN, 			walkN, 			walkN, 			walkN}}, 		//waitN
 {0x2, 0x0C,3,		{goE,				goE,			waitE,		waitE, 		wwaitE, 		wwaitE, 		wwaitE, 		wwaitE}}, 	// goE
 {0x2, 0x14, 2,		{goN,				goN,			goN,			goN, 			walkE, 			walkE, 			walkE, 			walkE}}, 		// waitE
 {0x2, 0x22, 2,		{walkE, 		walkE, 		walkE, 		walkE, 		walkE, 			walkE, 			walkE, 			walkE}}, 		//wwalkN
 {0x2, 0x14, 2,		{walkN, 		walkN, 		walkN, 		walkN, 		walkN, 			walkN, 			walkN, 			walkN}}, 		//wwalkE
 {0x8, 0x24, 1,		{warningE,	warningE,	warningE, warningE, warningE, 	warningE, 	warningE, 	warningE}}, // walkE
 {0x8, 0x24, 1,		{warningN,	warningN,	warningN, warningN, warningN, 	warningN, 	warningN, 	warningN}}, // walkN
 {0x2, 0x24, 1,		{goE, 			goE,			goN, 			goE, 			walkE, 			goE, 				goN, 				goE}}, 			//warningE
 {0x2, 0x24, 1,		{goN, 			goE,			goN, 			goN, 			walkN, 			goE, 				goN, 				goN}} 			//warningN
};

int main(void){
	volatile unsigned long delay;
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210); // activate traffic simulation and set system clock to 80 MHz
  SysTick_Init();     
  EnableInterrupts();

	//Initialize Ports B, E and F
	SYSCTL_RCGCGPIO_R |= 0x32;
	SYSCTL_RCGC2_R = 0x32;
  delay = 100; delay = 100; delay = 100; delay =100; delay = 100;
	//delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTB_DIR_R |= 0x3F;    // make PB5-0 out
  GPIO_PORTB_AFSEL_R &= ~0x3F; // disable alt func on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;    // enable digital I/O on PB5-0
                               // configure PB5-0 as GPIO
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF;
  GPIO_PORTB_AMSEL_R &= ~0x3F; // disable analog functionality on PB5-0
  GPIO_PORTE_DIR_R &= ~0x07;   // make PE1-0 in
  GPIO_PORTE_AFSEL_R &= ~0x07; // disable alt func on PE1-0
  GPIO_PORTE_DEN_R |= 0x07;    // enable digital I/O on PE1-0
                               // configure PE1-0 as GPIO
  GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFFFF000)+0x00000000;
  GPIO_PORTE_AMSEL_R &= ~0x07; // disable analog functionality on PE1-0
	
	GPIO_PORTF_DIR_R |= 0x0A;    // make PF1 and PF3 out
	GPIO_PORTF_AFSEL_R &= ~0x0A; // disable alt func on PF1 and PF3
  GPIO_PORTF_DEN_R |= 0x0A;    // enable digital I/O on PF1 and PF3
                               // configure PF1 and PF3 as GPIO
  GPIO_PORTF_PCTL_R &= ~0x0000F0F0;
  GPIO_PORTF_AMSEL_R &= ~0x0A; // disable analog functionality on PF1 and PF3

	Pt = goN; 
  while(1){
		LIGHT = Pt->Out;  // set lights
		SysTick_Wait10ms(Pt->Time);
		WLIGHT = Pt->OutF; // set lights for walk
		SysTick_Wait10ms(Pt->Time);
		Input = SENSOR;  // read sensors
		Pt = Pt->Next[Input]; 		
  }
}






