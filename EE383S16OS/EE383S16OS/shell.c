// shell.c  
// EE383 Spring 2016
// James E. lumpp Jr.  
// 4/9/2016
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "383os.h"
#include "Systick.h"

extern void delay(uint32_t);
void prmsg(char *);
int strcmp(const char *s1, const char *s2);

// -----------------------------------------------------------------
// SHELL BUILT_IN FUNCTIONS
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// FUNCTION  time:                                                  
//    Print the current time.                   
// ----------------------------------------------------------------- 

void time(void)
{  
unsigned char h,m,s;
 // Replace this with your print function or Unix time.
//  printf("\nThe time is: "); 
//  printf("%d:%d:%d\n",(int)h,(int)m,(int)s); // these variables will change in the background
	TIME_PrintEpochTime(EPOCH_SECONDS);
}

// -----------------------------------------------------------------
// FUNCTION  settime:                                                  
//    Prompt the user for the current time and enter into the globals                  
// ----------------------------------------------------------------- 

void settime(char *instr)
{  
//  int valid;

//  do{
//      printf("\n Set time to %s\n",instr);  // prompt user
// //     gets(str);  /repromt user?
//		  valid=1;
//  }while (valid==0);
	
	EPOCH_SECONDS = TIME_GetTime(instr);
//	EPOCH_SECONDS++;
//	delay(1);

}

void temp(void)
{  
int a;

 // Do analog to digital conversion and print the result

  printf("\nvalue is %d\n",a);  // request a single conversion
}
// -----------------------------------------------------------------
// Shell functions
// -----------------------------------------------------------------
// -----------------------------------------------------------------
// FUNCTION  parse:                                                  
//    This function replaces all white space with zeros until it     
// reaches a character that indicates the beginning of an     
// argument.  It saves the address to argv[].                   
// ----------------------------------------------------------------- 

void  parse(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */ 
          while (*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n' && *line != '\r') 
               line++;             /* skip the argument until ...    */
     }
     *argv = 0;                 /* mark the end of argument list  */
}

// -----------------------------------------------------------------                                             
//    This function will start a new process.  For now it just prints
//    The requested call                                        
// ----------------------------------------------------------------- 
     
void  execute(char **argv)
{ unsigned char i;

  printf("fork-exec: ");

  for(i=0;i<9;i++){
    if (argv[i] == 0) 
	   break;
    prmsg(argv[i]);
	  putchar(' ');
  }
  printf("\n");
}

void ps() {
	int taskCount = 1;
	
	int stackP, stackStart, stackEnd, stackPDif, stackEndDif, addr, perCPU;
	char *state;
	unsigned long stackSize;
	int perStk;
	TaskControlBlock *currentTask = getCurrentTask();
	TaskControlBlock *tempTask = currentTask;
	while((tempTask = tempTask->next) != currentTask) {
		taskCount++;
	}
	
	addr = (int) currentTask;
	stackStart = (int) currentTask->stack_start;
	stackEnd = (int) currentTask->stack_end;
	stackP = (int) currentTask->sp;
	stackSize = stackEnd - stackStart;
	stackPDif = stackP - stackStart;
	stackEndDif = stackEnd - stackStart;
	perCPU = 100 / taskCount;
	perStk = (stackPDif * 100) / stackEndDif;
	
	switch (currentTask->state) {
		case T_CREATED:
			state = "Created";
			break;
		case T_READY:
			state = "Ready";
			break;
		case T_RUNNING:
			state = "Running";
			break;
		default:
			state = "Unknown";
			break;
	}
	
	printf("%-10s%-5s%-10s%-10s%-10s%-10s%-15s\n", "USER", "TID", "%CPU", "STK_SZ", "%STK", "STATE", "ADDR");
	printf("%-10s%-5u%-10d%-10lu%-10d%-10s%#010x\n", "root", currentTask->tid, perCPU, stackSize, perStk, state, addr);
	
	tempTask = currentTask;
	while((tempTask = tempTask->next) != currentTask) {
		addr = (int) tempTask;
		stackStart = (int) tempTask->stack_start;
		stackEnd = (int) tempTask->stack_end;
		stackP = (int) tempTask->sp;
		stackSize = stackEnd - stackStart;
		stackPDif = stackP - stackStart;
		stackEndDif = stackEnd - stackStart;
		perCPU = 100 / taskCount;
		perStk = (stackPDif * 100) / stackEndDif;
	
		switch (tempTask->state) {
			case T_CREATED:
				state = "Created";
				break;
			case T_READY:
				state = "Ready";
				break;
			case T_RUNNING:
				state = "Running";
				break;
			default:
				state = "Unknown";
				break;
		}
		
		printf("%-10s%-5u%-10d%-10lu%-10d%-10s%#010x\n", "root", tempTask->tid, perCPU, stackSize, perStk, state, addr);
	}
}

void testSuspend() {
	printf("\n");
	ps();
	printf("\n");
	printf("suspended#");
	while (1) {
		
	}
}

void kill(char **argv) {
	
	int num;
	
	int errno = 0;
	long conv = strtol(argv[1], &argv[1], 10);

	// Check for errors: e.g., the string does not represent an integer
	// or the integer is larger than int
	if (errno != 0 || *argv[1] != '\0' || conv > NUM_TASKS) {
    // Put here the handling of the error, like exiting the program with
    // an error message
		printf("Usage: \"kill <TID>\"\nYou can't kill TID 0 or 1, and the TID must be active to kill it.\n");
	} else {
    // No error
    num = conv;    
    if (DeleteTask(num)) {
			printf("Killed process with TID %d\n", num);
		} else {
			printf("Failed to kill process with TID %d\n",num);
		}
	}
	
	
}

// -----------------------------------------------------------------
// implementation of 383 shell
// -----------------------------------------------------------------

void  shell(void)
{
     char  line[40] = {0};          /* the input line init all chars to zero  */
     char  *argv[10] = {0};              /* the command line argument      */
//	 unsigned char i;
     
     while (1) {                   /* repeat until done ....         */
          printf("383# ");     /*   display a prompt             */
		      gets(line);          // get a line from the user
          parse(line, argv);       /*   parse the line               */
          if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0 || strcmp(argv[0], "logout") == 0 ) {
		                   /* is it an "exit"?     */
                printf("[Process completed]\n");
				return;

				//exit(0);            
	      } else if (strcmp(argv[0], "time") == 0){
		      time();   //time(argv[1]);
				}else if (strcmp(argv[0], "settime") == 0){
		      settime(argv[1]);   //settime(argv[1]);
				}else if (strcmp(argv[0], "temp") == 0){
		      temp();   //temp(argv[1]);
				}else if (strcmp(argv[0], "ps") == 0){
					ps();
				}else if (strcmp(argv[0], "ts") == 0){
					unsigned char task_stack[128];
					CreateUFTask(testSuspend, task_stack, sizeof(task_stack));
				}else if (strcmp(argv[0], "kill") == 0){
					kill(argv);
				}else if (*argv[0] != 0 && argv[0] != 0){ 
					printf("\n-383: %s: command not found\n", argv[0]);
		      //execute(argv);    /* if not empty line execute command as new process*/
				}else{
					putchar('\n');
				}
     }//while(1)
}

// -----------------------------------------------------------------
// main function now just launches one shell
// -----------------------------------------------------------------


         
