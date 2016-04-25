// EE383 Spring 2015 LabA Main 
//  Author: James Lumpp
//  Date: 4/12/2016
//
// Pre-processor Directives
#include <stdio.h>  
#include <stdint.h> 
#include "UART.h"
#include "SysTick.h"
#include "PLL.h"
#include "383os.h"
#include "tm4c123gh6pm.h"
//#include "EdgeInterrupt.c" 

#define PF2             (*((volatile uint32_t *)0x40025010))

//  Global Declarations section
unsigned long EPOCH_SECONDS;

unsigned char task_zero_stack[MIN_STACK_SIZE]; // Declare a seperate stack 
unsigned char task_one_stack[MIN_STACK_SIZE];  // for each task
unsigned char task_two_stack[MIN_STACK_SIZE];
unsigned char task_shell_stack[1024];

void One(void);
void Two(void);
void Zero(void);
void software_delay_halfsecond(void);

// Function Prototypes
void shell(void);

void Zero(void)
	{
	//while(1) 
	//   putchar('0');
  //tasks should not end
	
	
	 while(1) 
	{   
		software_delay_halfsecond();               
		PF2 ^= 0x04;     // toggle PF2 (Blue LED) 
	} 
	
	}

void One(void)
	{

	//while(1)  
	//	putchar('1');
	
 while(1) 
	{   
		software_delay_halfsecond();               
		PF2 ^= 0x04;     // toggle PF2 (Blue LED) 
	}  
	
	} // end the 
	
void Two(void)
	{

	//while (1) 
	//	putchar('2');
  //tasks should not end
	
	 while(1) 
	{   
		software_delay_halfsecond();               
		PF2 ^= 0x04;     // toggle PF2 (Blue LED) 
	} 
	
} 
	
	

// main
int main(void) {
	
	PLL_Init();     // 50 MHz (SYSDIV2 == 7, defined in pll.h)
  UART_Init();    // initialize UART
	                //115,200 baud rate (assuming 50 MHz UART clock),
                  // 8 bit word length, no parity bits, one stop bit, FIFOs enabled
	SysTick_Init();
	
	
//PortF_Init();
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
	GPIO_PORTF_DIR_R |= 0x04;             // make PF2 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;          // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;             // enable digital I/O on PF2
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  //GPIO_PORTF2 = 0;                      // turn off LED
	
  puts("\n\nWelcome to the EE383 Operating System...\n\n");

	// Create tasks that will run (these are functions that do not return)
	
	CreateTask(shell, task_shell_stack, sizeof (task_shell_stack));
	CreateTask(Zero, task_zero_stack, sizeof (task_zero_stack));
	CreateTask(One, task_one_stack, sizeof (task_one_stack));
	CreateTask(Two, task_two_stack, sizeof (task_two_stack));
	
	puts("\nStarting Scheduler...");
	
	StartScheduler();  //Start the OS Scheduling, does not return
	
	while(1) // should never get here, but just in case...
		;
} //main







