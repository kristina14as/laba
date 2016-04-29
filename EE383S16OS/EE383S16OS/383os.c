// EE383 OS
// Fall 2016
// James E. Lumpp, Jr.
// 4/11/16

#include <stdlib.h>
#include <stdint.h>
#include "383os.h"
#include <stdio.h>

static TaskControlBlock task_list[NUM_TASKS], *TASK_LIST_PTR;
static TaskControlBlock *CURRENT_TASK;
static TaskControlBlock *SHELL_TASK;
static TaskControlBlock *UF_TASK; //the task running in place of the shell, IF THERE IS ONE

static int NEXT_TID;
static unsigned char null_task_stack[60];  // is not used, null task uses original system stack
static void InitSystem(void);
static void NullTask(void);
void EdgeCounter_Init(void);

TaskControlBlock* getCurrentTask(void) {
	return CURRENT_TASK;
}

TaskControlBlock* getShellTask(void) {
	return SHELL_TASK;
}

TaskControlBlock* getUFTask(void) {
		return UF_TASK;
}

TaskControlBlock* getTask(int tid) {
	int i;
		TaskControlBlock* temp = getCurrentTask();
		TaskControlBlock* currentTask = getCurrentTask();
	for (i = 0; i < NUM_TASKS; i++) {
		if (temp->tid == tid) {
				return temp;
		} else {
			temp = temp->next;
		}
	}
	//if we get here then we couldn't find the task
	return 0;
}

void setUFTask(TaskControlBlock *newUFTask) {
		UF_TASK = newUFTask;
}

TaskControlBlock* getNullTask(void) {
	int i;
	TaskControlBlock* task = getCurrentTask();
	for (i=0; i<NUM_TASKS; i++) {
		if (task->tid == 0) {
			return task;
		}
		task = task->next;
	}
	printf("ERROR on GetNullTask");
	return task;
}


TaskControlBlock* getPreviousTask(TaskControlBlock *task) {
	int i;
	TaskControlBlock *tempTask = task;
	for( i=0; i < NUM_TASKS; i++) {
		if (tempTask->next == task) {
			return tempTask;
		}
		//otherwise, keep going
		tempTask = tempTask->next;
	}
	//if we get here something fucky happened
	printf("ERROR on getPreviousTask");
	return task;
}

bool isShellSuspended(void) {
	int i;
	TaskControlBlock* null = getNullTask();
	TaskControlBlock* temp = getNullTask()->next;
	for(i = 0; i < NUM_TASKS; i++) {
		if (temp == SHELL_TASK) {
			return false;
		}
		//keep going
		temp = temp->next;
	}
	return true;
}
	
	

            /* Start the multi-tasking system */
int StartScheduler(void)
	{
	if (CURRENT_TASK == NULL)
		return -1;

	EdgeCounter_Init();           // initialize GPIO Port F interrupt OR SysTick OR ...
	
  NullTask();                   // Will not return
	return 0;	 
	}

void ResumeShellTask() {
	
	TaskControlBlock* nullTask = getNullTask();
	TaskControlBlock* prevTask = getPreviousTask(nullTask);
	TaskControlBlock* shell = getShellTask();
	if (shell == 0 || prevTask == 0) {
			printf("Resuming shell task failed");
	}
//	if (!isShellSuspended()) {
//		//the shell is already running!
//		printf("The shell is already running!\n");
//		return;
//	}
	
	prevTask->next = shell;
	shell->next = nullTask;
	//nullTask->next = shell;
	
	
}	
	
void SuspendShellTask() {
	//first, get a reference to the task before and the task after the shell in the tasks array
	TaskControlBlock* previous = getPreviousTask(getShellTask());
	TaskControlBlock* next = getShellTask()->next;
	//skip the shell task
	previous->next = next;
}

void handleETX(void) {
	printf("\n383# ");
	DeleteUFTask();
	ResumeShellTask();
}

void DeleteUFTask(void) {
	//first, get a reference to the task before and the task after the UF in the tasks array
	TaskControlBlock* previous = getPreviousTask(getUFTask());
	TaskControlBlock* next = getUFTask()->next;
	if (previous == 0 || next == 0) {
			printf("Deleting UF task failed");
	}
	//skip the UF task
	previous->next = next;
	//TASK_LIST_PTR = getUFTask();
	//NEXT_TID--;
}

bool DeleteTask(int tid) {
		if(tid != 0 && tid != 1) { //refuse to delete null or shell
				TaskControlBlock* desiredTask = getTask(tid);
				if (desiredTask != 0) { //if we actually got the task
					TaskControlBlock* previous = getPreviousTask(desiredTask);
					TaskControlBlock* next = desiredTask->next;
					previous->next = next;
					return true;
				} else {
					return false;
				}
		} else {
				return false;
		}
}

int CreateShellTask(void (*func)(void), 
                    unsigned char *stack_start,
                    unsigned stack_size)
		{
				int tid = CreateTaskImpl((*func), 
                    stack_start,
                    stack_size, true, false);							
				return tid;							
		}
		
int CreateUFTask(void (*func)(void), 
                    unsigned char *stack_start,
                    unsigned stack_size)
		{
			//create the task stack in here
			
				int tid = CreateTaskImpl((*func), 
                    stack_start,
                    stack_size, false, true);
				//creating a user-facing task automatically suspends the shell
				SuspendShellTask();
				return tid;							
		}
	
/* Create a new process and link it to the task list
 */
int CreateTask(void (*func)(void), 
                    unsigned char *stack_start,
                    unsigned stack_size)
					//,unsigned ticks)
	{
		int tid = CreateTaskImpl((*func), stack_start, stack_size, false, false);
		return tid;
	}
	
int CreateTaskImpl(void (*func)(void), 
                    unsigned char *stack_start,
                    unsigned stack_size, bool isShell, bool isUF)
										{
											//	long ints;
	TaskControlBlock *p, *next;

	if (TASK_LIST_PTR == 0)
		InitSystem();
	
//	ints=StartCritical();
	p = TASK_LIST_PTR;
	TASK_LIST_PTR = TASK_LIST_PTR->next;
	p->func = func;
	p->state = T_CREATED;
	p->tid = NEXT_TID++;

	       /* stack grows from high address to low address */
	p->stack_start = stack_start;
	p->stack_end = stack_start+stack_size-1;
	
	p->sp = p->stack_end;

	           /* create a circular linked list */
	if (CURRENT_TASK == NULL) {
		p->next = p, CURRENT_TASK = p;
	} else {
		next = CURRENT_TASK->next, CURRENT_TASK->next = p, p->next = next;
	}
	
	if (isShell) {
		//special shell handling
		//we are assuming there is only one shell at a time
		//if this is called and there is already a shell, 
		SHELL_TASK = p;
	}
	if (isUF) {
		UF_TASK = p;
	}
//  EndCritical(ints);
	return p->tid;			
										}
/* Initialize the system.
 */
static void InitSystem(void)
	{
	int i;

	         /* initialize the free list  */
	for (i = 0; i < NUM_TASKS-1; i++)
		task_list[i].next = &task_list[i+1];
	TASK_LIST_PTR = &task_list[0];

	         /* null task has tid of 0 */
	CreateTask(NullTask, null_task_stack, sizeof (null_task_stack));
	}


/* Always runnable task. This has the tid of zero
 */
static void NullTask(void)
	{

	while (1) 
		;          //  putchar('n');
	 
	}


// Schedule will save the current SP and then call teh scheduler
//	SHOULD ONLY BE CALLED IN ISR
/* Schedule(): Run a different task. Set the current task as the next one in the (circular)
 * list, then set the global variables and call the appropriate asm routines
 * to do the job. 
 */
unsigned char * Schedule(unsigned char * the_sp)  
	{
		unsigned char * sp;
               // save the current sp and schedule
	 CURRENT_TASK->sp = the_sp;
	 CURRENT_TASK->state = T_READY;
	 CURRENT_TASK = CURRENT_TASK->next;

	 if (CURRENT_TASK->state == T_READY){
		  CURRENT_TASK->state = T_RUNNING;
	    sp = CURRENT_TASK->sp;    
	 } else {     /* task->state == T_CREATED so make it "ready" 
	                give it an interrupt frame and then launch it 
	    		        (with a blr sith 0xfffffff9 in LR in StartNewTask())  */
		  CURRENT_TASK->state = T_RUNNING;
			sp = StartNewTask(CURRENT_TASK->sp,(uint32_t) CURRENT_TASK->func); // Does not return!
		}
		return(sp);
	}
