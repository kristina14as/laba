// EE383 Spring 2015 LabA Main 
//  Author: James Lumpp
//  Date: 4/12/2016
//
// Pre-processor Directives
#include <stdio.h>  
#include <stdint.h> 
#include "UART.h"
//#include "SysTick.h"
#include "PLL.h"
#include "383os.h"
#include "tm4c123gh6pm.h"
#include <stdbool.h>
//#include "EdgeInterrupt.c" 

#define PF2             (*((volatile uint32_t *)0x40025010))
#define LED_red    0x02 //  R--    0x02
#define LED_blue   0x04 //  --B    0x04
#define LED_green  0x08 //  -G-    0x08

//  Global Declarations section
typedef enum {GREEN, RED, BLUE} led_color_type;
//unsigned long EPOCH_SECONDS;

unsigned char task_zero_stack[MIN_STACK_SIZE]; // Declare a seperate stack 
unsigned char task_one_stack[MIN_STACK_SIZE];  // for each task
unsigned char task_two_stack[MIN_STACK_SIZE];

unsigned char task_shell_stack[1024];

void PortF_Init(void);
void One(void);
void Two(void);
void Zero(void);
void software_delay_halfsecond(void);

void toggle_led(led_color_type lightColor);
void delay(unsigned long time);

// Function Prototypes
void shell(void);
void testSuspend(void);

void Zero(void)
	{
	//while(1) 
	 //  putchar('0');
  //tasks should not end
	
	
	 while(1) 
	{   
		//delay(1);
		//putchar('0');
		toggle_led(BLUE);
	} 
	
	}

void One(void)
	{

	//while(1)  
		//putchar('1');
	
 while(1) 
		{   
		//delay(1);
		//putchar('1');			
		toggle_led(RED);
	}  
	
	} // end the 
	
void Two(void)
	{

	//while (1) 
	//	putchar('2');
  //tasks should not end
	
	while(1) 
	{   
		//delay(1);
		//putchar('2');
		toggle_led(GREEN);
	} 
	
}
	
void toggle_led(led_color_type lightColor) {
	switch (lightColor) {
			case RED:
					GPIO_PORTF_DATA_R &= ~LED_green; 
					GPIO_PORTF_DATA_R &= ~LED_blue; 
					GPIO_PORTF_DATA_R ^= LED_red;
				break;
			case BLUE:
				GPIO_PORTF_DATA_R &= ~LED_green; 
					GPIO_PORTF_DATA_R &= ~LED_red; 
					GPIO_PORTF_DATA_R ^= LED_blue;
				break;
			case GREEN:
					GPIO_PORTF_DATA_R &= ~LED_red; 
					GPIO_PORTF_DATA_R &= ~LED_blue; 
					GPIO_PORTF_DATA_R ^= LED_green;
			default:
				break;
		}
}

// main
int main(void) {
	PortF_Init();
	PLL_Init();     // 50 MHz (SYSDIV2 == 7, defined in pll.h)
  UART_Init();    // initialize UART
	                //115,200 baud rate (assuming 50 MHz UART clock),
                  // 8 bit word length, no parity bits, one stop bit, FIFOs enabled
	SysTick_Init();
	
	
////PortF_Init();
//	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
//	GPIO_PORTF_DIR_R |= 0x0E;             // make PF2 out (built-in LED)
//  GPIO_PORTF_AFSEL_R &= ~0x04;          // disable alt funct on PF2
//  GPIO_PORTF_DEN_R |= 0x1F;             // enable digital I/O on PF2
//                                        // configure PF2 as GPIO
//  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
//  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
//  //GPIO_PORTF2 = 0;                      // turn off LED
	
  puts("\n\nWelcome to the EE383 Operating System...\n\n");

	// Create tasks that will run (these are functions that do not return)
	
	CreateShellTask(shell, task_shell_stack, sizeof (task_shell_stack));
	
	//CreateTask(Zero, task_zero_stack, sizeof (task_zero_stack));
	//CreateTask(One, task_one_stack, sizeof (task_one_stack));
	//CreateTask(Two, task_two_stack, sizeof (task_two_stack));
	
	puts("\nStarting Scheduler...");
	
	StartScheduler();  //Start the OS Scheduling, does not return
	
	while(1) // should never get here, but just in case...
		;
} //main

void PortF_Init(void){ 
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}




