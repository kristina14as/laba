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
#include <stdbool.h>
//#include "EdgeInterrupt.c" 

#define PF2             (*((volatile uint32_t *)0x40025010))
#define LED_red    0x02 //  R--    0x02
#define LED_blue   0x04 //  --B    0x04
#define LED_green  0x08 //  -G-    0x08

//  Global Declarations section
typedef enum {GREEN, RED, BLUE} led_color_type;
unsigned long EPOCH_SECONDS;

unsigned char task_zero_stack[MIN_STACK_SIZE]; // Declare a seperate stack 
unsigned char task_one_stack[MIN_STACK_SIZE];  // for each task
unsigned char task_two_stack[MIN_STACK_SIZE];
unsigned char task_shell_stack[1024];

void PortF_Init(void);
void One(void);
void Two(void);
void Zero(void);
void software_delay_halfsecond(void);
int32_t TIME_GetTime(void);
void TIME_PrintEpochTime(int32_t epoch_time);
int32_t TIME_EncodeEpoch( uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second);
void toggle_led(led_color_type lightColor);
void delay(unsigned long time);

// Function Prototypes
void shell(void);

void Zero(void)
	{
	//while(1) 
	//   putchar('0');
  //tasks should not end
	
	
	 while(1) 
	{   
		SysTick_Wait10ms(1);
		putchar('0');
		GPIO_PORTF_DATA_R ^= LED_blue;
	} 
	
	}

void One(void)
	{

	//while(1)  
	//	putchar('1');
	
 while(1) 
		{   
		SysTick_Wait10ms(1);  
		putchar('1');			
		GPIO_PORTF_DATA_R ^= LED_red;
	}  
	
	} // end the 
	
void Two(void)
	{

	//while (1) 
	//	putchar('2');
  //tasks should not end
	
	while(1) 
	{   
		SysTick_Wait10ms(1);
		putchar('2');
		GPIO_PORTF_DATA_R ^= LED_green;
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
void toggle_led(led_color_type lightColor){
		switch (lightColor) {
			case RED:
					GPIO_PORTF_DATA_R ^= LED_red;
				break;
			case BLUE:
					GPIO_PORTF_DATA_R ^= LED_blue;
				break;
			case GREEN:
					GPIO_PORTF_DATA_R ^= LED_green;
			default:
				break;
		}
}
	
	
//	GPIO_PORTF_DIR_R |= 0x0E;             // make PF2 out (built-in LED)
//  GPIO_PORTF_AFSEL_R &= ~0x04;          // disable alt funct on PF2
//  GPIO_PORTF_DEN_R |= 0x1F;             // enable digital I/O on PF2
//                                        // configure PF2 as GPIO
//  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
//  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
//  //GPIO_PORTF2 = 0;                      // turn off LED
	
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

int32_t TIME_GetTime()
{
//	int32_t new_time;
	char input[14];
	int year, month, day, hour, minute, second;
	int ok_date = 0;   // flag that gets set to true once an acceptable date is input by the user
	
	//loop until the user enters an acceptable date
	while(ok_date == 0)
	{
		printf("\nThis loop will go until you enter a valid year/month/day/hour/min/sec");
		printf("\nPlease enter time as YYYYMMDDHHMISS: ");
		UART_InString(input, 14);
		//gets(input);   //get the input from the user and store in an array of character
		
		sscanf(input, "%4d%2d%2d%2d%2d%2d\n", &year, &month, &day, &hour, &minute, &second);         ///%d%d%d%d%d%d", &)
		
		if((month > 12) || (month == 0) || (day>31) || (day == 0) || (hour >23) || (hour<=0) || (minute>59) || (minute<0) || (second>59) || (second < 0))
		{
			ok_date = 0;
			printf("\nThe number that you entered was NOT a valid year/month/day/hour/min/sec");
		}
		else
			ok_date = 1;
	
	}// end of loop that checks for an acceptable date
	
	
	// prints the six integers
	//printf("\n%4u %2u %2u %2u %2u %2u\n",year,month,day,hour,minute,second);
	printf("\nyear = %d, month = %d, day = %d, hours = %d, minutes = %d, seconds = %d\n",year,month,day,hour,minute,second);
	
	
	///////////this is where you convert the input time 
	
	return TIME_EncodeEpoch(year, month, day, hour, minute, second);
}// end of GetTime


// converts the given Epoch seconds into the 6 integers: year, month, day, hour, minute, second
//// assume valid epoch time is passed in
void TIME_PrintEpochTime(int32_t epoch_time)  // IS THIS SUPPOSED TO BE PRINT EPOCH TIME?
{
	uint32_t year, month, day, hour, minute, second;
	uint32_t leap_days;
	//int num_months[12];
	epoch_time = epoch_time + 86400; /////////ADD A DAY
	printf("\nThe total time in EPOCH seconds is: %u", epoch_time);
	
		// year = (total time / # secs in year) + 1970
	year = (epoch_time/31536000) + 1970;				
	//Determine number of leap days
	leap_days = (year - 1972)/4 + 1;
	//Determine if date is in a leap year, and if has occurred
	if ((year%4 == 0) && (month <= 2))
	{		//minus 1 because this year's leap day hasn't happened yet
				leap_days--;			
	}
	// Subtract all leap days in seconds
	epoch_time -= (leap_days*86400);			
	// now get the real number of years and take out of epoch
	year = ((epoch_time/31536000) + 1970);
	epoch_time -= (31536000*(year - 1970));
	
	// check the month
	if((epoch_time/28857600)==1)
	{month = 12;}
	else if((epoch_time/26265600)==1)
	{		month = 11;}
	else if((epoch_time/23587200)==1)
	{		month = 9;}
	else if((epoch_time/20995200)==1)
	{		month = 8;}
	else if((epoch_time/18316800)==1)
	{		month = 7;}
	else if((epoch_time/13046400)==1)
	{		month = 6;}
	else if((epoch_time/10368000)==1)
	{		month = 5;}
	else if((epoch_time/7776000)==1)
	{		month = 4;}
	else if((epoch_time/5097600)==1)
	{		month = 3;}
	else if((epoch_time/2678400)==1)
	{		month = 2;}
	else
		month = 1;
	
	switch (month){
case 1: epoch_time -= 0;break;
case 2: epoch_time -= 2678400;break;
case 3: epoch_time -= 5097600;break;
case 4: epoch_time -= 7776000;break;
case 5: epoch_time -= 10368000;break;
case 6: epoch_time -= 13046400;break;
case 7: epoch_time -= 15638400;break;
case 8: epoch_time -= 18316800;break;
case 9: epoch_time -= 20995200;break;
case 10: epoch_time -= 23587200;break;
case 11: epoch_time -= 26265600;break;
case 12: epoch_time -= 28857600;break;
}
//Add in fully completed days
//epoch_seconds += (day - 1)*86400
day = ((epoch_time/86400)+1);
epoch_time -= ((day - 1)*86400);

//Add in fully completed hours
//No -1 because hour starts at 0
//epoch_seconds += hour*3600
hour = (epoch_time/3600);
epoch_time -= (hour*3600);

//Add in fully completed minutes
//epoch_seconds := minute*60
//epoch_seconds += second;
minute = (epoch_time/60);
epoch_time -= (minute*60);

second = epoch_time;
//epoch_time -= second;

//printf("\n Your date in regular notation is: %4u%2u%2u%2u%2u%2u", year, month, day, hour, minute, second);
printf("\nYour Regular Date from the EPOCH time is: year: %4u month: %2u day: %2u hour: %2u minute: %2u second: %2u\n", year, month, day, hour, minute, second);
	
}// end of PrintTime


// for part 5 - converts the given date into UNIX Epoch seconds
int32_t TIME_EncodeEpoch( uint32_t year,
uint32_t month,
uint32_t day,
uint32_t hour,
uint32_t minute,
uint32_t second)
{
	int32_t epoch_seconds;
	int32_t leap_days;
	//Pseudo Code for TIME_EncodeEpoch()
	epoch_seconds = 31536000*(year - 1970); // seconds per year
	//Determine number of leap days
	leap_days = (year - 1972)/4 + 1;
	//Determine if date is in a leap year, and if has occurred
	if (year%4 == 0)
	{
		if (month <= 2)
		leap_days--;
			//minus 1 because this year's leap day hasn't happened yet
	}
	epoch_seconds += leap_days*86400;
	
	//Fully completed months
switch (month){
case 1: epoch_seconds += 0;break;
case 2: epoch_seconds += 2678400;break;
case 3: epoch_seconds += 5097600;break;
case 4: epoch_seconds += 7776000;break;
case 5: epoch_seconds += 10368000;break;
case 6: epoch_seconds += 13046400;break;
case 7: epoch_seconds += 15638400;break;
case 8: epoch_seconds += 18316800;break;
case 9: epoch_seconds += 20995200;break;
case 10: epoch_seconds += 23587200;break;
case 11: epoch_seconds += 26265600;break;
case 12: epoch_seconds += 28857600;break;
}
//Add in fully completed days
epoch_seconds += (day - 1)*86400;

//Add in fully completed hours
//No -1 because hour starts at 0
epoch_seconds += hour*3600;

//Add in fully completed minutes
epoch_seconds += minute*60;
epoch_seconds += second;


return(epoch_seconds); //Function TIME_EncodeEpoch()
	
}// end of TIME_EncodeEpoch

//number of half seconds to delay
void delay(unsigned long time){
  unsigned long i;
  while(time > 0){
    i = 266667;
    while(i > 0){
      i = i - 1;
    }
    time = time - 1;
  }
}





