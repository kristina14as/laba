// SysTick.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize the SysTick module, wait at least a
// designated number of clock cycles, and wait approximately a multiple
// of 10 milliseconds using busy wait.  After a power-on-reset, the
// LM4F120 gets its clock from the 16 MHz precision internal oscillator,
// which can vary by +/- 1% at room temperature and +/- 3% across all
// temperature ranges.  If you are using this module, you may need more
// precise timing, so it is assumed that you are using the PLL to set
// the system clock to 50 MHz.  This matters for the function
// SysTick_Wait10ms(), which will wait longer than 10 ms if the clock is
// slower.
// Daniel Valvano
// September 11, 2013

/* This example accompanies the books
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
   Volume 1, Program 4.7
   
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Program 2.11, Section 2.6

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
#include "tm4c123gh6pm.h"
#include "UART.h"
#include <stdio.h>  
#include <stdint.h> 
//#include "Systick.h"


#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value
#define PD0 (*((volatile uint32_t *)0x40007004))
#define PF2             (*((volatile uint32_t *)0x40025010))
	

extern unsigned long EPOCH_SECONDS;
extern unsigned long COUNT;


// Initialize SysTick with busy wait running at bus clock.
//void SysTick_Init(void){
//  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
// NVIC_ST_RELOAD_R = NVIC_ST_RELOAD_M;  // maximum reload value
//  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
//                                        // enable SysTick with core clock
//  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC;
//}

volatile uint32_t Counts;
void SysTick_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x08;		//activate port D
	COUNT = 0;
//	GPIO_PORTD_AMSEL_R &= ~0x01;		//no analog
//	GPIO_PORTD_PCTL_R &= ~0x0000000F;	//regular GPIO function
//	GPIO_PORTD_DIR_R |= 0x01;   // make PD0 out
//	GPIO_PORTD_AFSEL_R &= ~0x01;// disable alt funct on PD0
//  GPIO_PORTD_DEN_R |= 0x01;   // enable digital I/O on PD0
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
	NVIC_ST_RELOAD_R = 5000000-1;// reload value // results in a 10 ms period for part 1
	NVIC_ST_CURRENT_R = 0;      // any write to current clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2   
	NVIC_ST_CTRL_R = 0x00000007; // enable SysTick with core clock and interrupts
  EnableInterrupts();
}//end New SysTick_Init




// Time delay using busy wait.
// The delay parameter is in units of the core clock. (units of 20 nsec for 50 MHz clock)
void SysTick_Wait(unsigned long delay){
  volatile uint32_t elapsedTime;
  uint32_t startTime = NVIC_ST_CURRENT_R;
  do{
    elapsedTime = (startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);
}
// Time delay using busy wait.
// This assumes 50 MHz system clock.
void SysTick_Wait10ms(unsigned long delay){
  uint32_t i;
  for(i=0; i<delay; i++){
    SysTick_Wait(500000);  // wait 10ms (assumes 50 MHz clock)
  }
}



// for part 5 - converts the given date into UNIX Epoch seconds
int32_t TIME_EncodeEpoch( uint32_t year,
uint32_t month,
uint32_t day,
uint32_t hour,
uint32_t minute,
uint32_t second)
{
	int32_t epoch_sec;
	int32_t leap_days;
	//Pseudo Code for TIME_EncodeEpoch()
	epoch_sec = 31536000*(year - 1970); // seconds per year
	//Determine number of leap days
	leap_days = (year - 1972)/4 + 1;
	//Determine if date is in a leap year, and if has occurred
	if (year%4 == 0)
	{
		if (month <= 2)
		leap_days--;
			//minus 1 because this year's leap day hasn't happened yet
	}
	epoch_sec += leap_days*86400;
	
	//Fully completed months
switch (month){
case 1: epoch_sec += 0;break;
case 2: epoch_sec += 2678400;break;
case 3: epoch_sec += 5097600;break;
case 4: epoch_sec += 7776000;break;
case 5: epoch_sec += 10368000;break;
case 6: epoch_sec += 13046400;break;
case 7: epoch_sec += 15638400;break;
case 8: epoch_sec += 18316800;break;
case 9: epoch_sec += 20995200;break;
case 10: epoch_sec += 23587200;break;
case 11: epoch_sec += 26265600;break;
case 12: epoch_sec += 28857600;break;
}
//Add in fully completed days
epoch_sec += (day - 1)*86400;

//Add in fully completed hours
//No -1 because hour starts at 0
epoch_sec += hour*3600;

//Add in fully completed minutes
epoch_sec += minute*60;
epoch_sec += second;


return(epoch_sec); //Function TIME_EncodeEpoch()
	
}// end of TIME_EncodeEpoch

//number of half seconds to delay
//do not use
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





int32_t TIME_GetTime(char *in)
{
//	int32_t new_time;
//	char input[14];
	int year, month, day, hour, minute, second;
	int ok_date = 0;   // flag that gets set to true once an acceptable date is input by the user
	
	//loop until the user enters an acceptable date
	//while(ok_date == 0)
//	{
//		printf("\nThis loop will go until you enter a valid year/month/day/hour/min/sec");
//		printf("\nPlease enter time as YYYYMMDDHHMISS: ");
		//UART_InString(input, 14);
		//gets(input);   //get the input from the user and store in an array of character
		
		sscanf(in, "%4d%2d%2d%2d%2d%2d\n", &year, &month, &day, &hour, &minute, &second);         ///%d%d%d%d%d%d", &)
		
		if((month > 12) || (month == 0) || (day>31) || (day == 0) || (hour >23) || (hour<=0) || (minute>59) || (minute<0) || (second>59) || (second < 0))
		{
			ok_date = 0;
			printf("\nThe number that you entered was NOT a valid year/month/day/hour/min/sec\n");
		}
		else{
			ok_date = 1;
		}
	
//	}// end of loop that checks for an acceptable date
	
	if (ok_date){
	// prints the six integers
	//printf("\n%4u %2u %2u %2u %2u %2u\n",year,month,day,hour,minute,second);
	printf("\nyear = %d, month = %d, day = %d, hours = %d, minutes = %d, seconds = %d\n",year,month,day,hour,minute,second);
	
	}
	
	///////////this is where you convert the input time 
	
	return TIME_EncodeEpoch(year, month, day, hour, minute, second);
			
	
		
		
}// end of GetTime



// converts the given Epoch seconds into the 6 integers: year, month, day, hour, minute, second
//// assume valid epoch time is passed in
void TIME_PrintEpochTime(uint32_t epoch_seconds)
{
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
	uint32_t leap_days; // initialize
	
	year = (epoch_seconds/31536000) + 1970; // finding the year so the leap days can be found
	
	leap_days = (year - 1972)/4 + 1; // find number of leap days
	
//	if (year%4 == 3)
//	{
//		leap_days--;
//	}
	//Determine if date is in a leap year, and if has occurred
	if ((year%4 == 0))
	{
		if (month >= 2)
		{
			leap_days--;
			//minus 1 because this year's leap day hasn't happened yet
		}
	}
	
	epoch_seconds -= leap_days*86400;  // subtract leapdays from epoch_seconds
	
	year = (epoch_seconds/31536000) + 1970; // find the year after the leap days have been removed
	epoch_seconds -= 31536000*(year - 1970); // subtract the year from epoch_seconds
	
	
	if ((epoch_seconds / 28857600) == 1 )  // if the month is december
	{
		month = 12;
		epoch_seconds = epoch_seconds - 28857600;
	}
	else if ((epoch_seconds / 26265600) == 1)  // if the month is november
	{
		month = 11;
		epoch_seconds = epoch_seconds - 26265600;
	}
	else if ((epoch_seconds / 23587200) == 1)  // if the month is october
	{
		month = 10;
		epoch_seconds = epoch_seconds - 23587200;
	}	
	else if ((epoch_seconds / 20995200) == 1)  // if the month is september
	{
		month = 9;
		epoch_seconds = epoch_seconds - 20995200;
	}
	else if ((epoch_seconds / 18316800) == 1)  // if the month is august
	{
		month = 8;
		epoch_seconds = epoch_seconds - 18316800;
	}
	else if ((epoch_seconds / 15638400) == 1)  // if the month is july
	{
		month = 7;
		epoch_seconds = epoch_seconds - 15638400;
	}
	else if ((epoch_seconds / 13046400) == 1)  // if the month is june
	{
		month = 6;
		epoch_seconds = epoch_seconds - 13046400;
	}
	else if ((epoch_seconds / 10368000) == 1)  // if the month is may
	{
		month = 5;
		epoch_seconds = epoch_seconds - 10368000;
	}
	else if ((epoch_seconds / 7776000) == 1)  // if the month is april
	{
		month = 4;
		epoch_seconds = epoch_seconds - 7776000;
	}
	else if ((epoch_seconds / 5097600) == 1) // if the month is march
	{
		month = 3;
		epoch_seconds = epoch_seconds - 5097600;
	}
	else if ((epoch_seconds / 2678400) == 1)  // if the month is febuary
	{
		month = 2;
		epoch_seconds = epoch_seconds - 2678400;
	}
	else   // if the month is january
	{
		month = 1;
	}

	//find days
	day = (epoch_seconds/86400) + 1;
	//printf("%2u", epoch_seconds);
	//printf("%2u", day);
	epoch_seconds -= (day - 1)*86400;
	
	//find hours
	//No -1 because hour starts at 0
	hour = (epoch_seconds/3600);
	epoch_seconds -= hour*3600;
	
	//find minutes
	minute = epoch_seconds / 60;
	epoch_seconds -= minute*60;
	
	// find seconds
	second = epoch_seconds;
	
	// output the date
	printf("\nEpoch Date: %4u %2u %2u %2u %2u %2u\n", year, month, day, hour, minute, second);
	
	
}





