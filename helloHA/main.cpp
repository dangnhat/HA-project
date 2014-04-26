#include <cstdio>
#include "MB1_System.h"

extern "C" {
#include "kernel.h"
#include "thread.h"
}

/* global vars */
int glb_aFlag = 0;
int threadA_pid, threadB_pid;

/* new threads (A and B) */
char threadA_stack [KERNEL_CONF_STACKSIZE_PRINTF];
char threadB_stack [KERNEL_CONF_STACKSIZE_PRINTF];

void threadA_func (void){
	while (1){
		printf ("This is thread %s\n", thread_getname (thread_getpid ()) );
		printf ("glb_aFlag : %d\n", glb_aFlag++);
		delay_ms (1000);

		//thread_yield ();
	}
}

void threadB_func (void){
	while (1){
		printf ("This is thread %s\n", thread_getname (thread_getpid ()) );
		printf ("glb_aFlag : %d\n", glb_aFlag++);
		delay_ms (1000);

		//thread_yield ();
	}
}

int main (void){
	
	MB1_system_init ();

	printf ("Welcome to HLib's MBoard-1 \n");

	/* create new threads A and B */
	threadA_pid = thread_create (threadA_stack, sizeof (threadA_stack), 0, CREATE_WOUT_YIELD, threadA_func, "Thread A");
	if (threadA_pid < 0)
		printf ("failed to create thread A\n");

	threadB_pid = thread_create (threadB_stack, sizeof (threadA_stack), 0, 0, threadB_func, "Thread B");
	if (threadB_pid < 0)
		printf ("failed to create thread B\n");

	//sched_context_switch_request = 1;

	while (1){
		printf ("This is thread main\n");
		MB1_Led_green.toggle ();
		delay_ms (1000);
	}
	
	return 0;
}
