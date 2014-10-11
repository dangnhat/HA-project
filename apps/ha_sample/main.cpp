/*
 * Copyright (C) 2014 Hamburg University of Applied Sciences (HAW)
 * Copyright (C) 2014 Ho Chi Minh University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @file        main.cpp
 * @brief       Demonstration of mixed c++ and c user application with pure c RIOT
 *              - mixing of c and c++ source to test name mangling
 *              - introducing a namespace to declarative block, avoiding to qualify calls, e.g. std::vector
 *              - using private and public member functions, e.g. 'cpp_obj.greet()' cannot be accessed from main.cpp
 *              - overloading of function 'cpp_obj.say_hello(...)' for 'none', 'int' or 'float'
 *              - demonstration of templated c++ container 'std::vector'
 *              - usage of iterator to access elements of the container type
 *
 * @author      Martin Landsmann <martin.landsmann@haw-hamburg.de>
 * @author      DangNhat Pham-Huu <51002279@hcmut.edu.vn>
 */

/*
 * all included headers defining c functions, i.e. all RIOT functions, must be marked as extern "C"
 */
extern "C" {
#include "thread.h"

#include "c_functions.h"
}

#include <cstdio>
#include <vector>
#include "cpp_class.h"
#include "MB1_System.h"

#include "ble_transaction.h" //used by ANH

using namespace Btn_ns;

using namespace std;

/*******************************************************************************
 * Variables & Data Buffer
 *
 ******************************************************************************/
uint8_t 			rxBuf[MAX_BUF_SIZE]		= "\0";	// hold usart3 rx data
uint8_t 			index					= 0;
ble_serv_stt_s 		BTFlags;
uint8_t 			msgBuf[]				=  "This is default data of BLE112A";
uint8_t				attBuf[MAX_MSGBUF_SIZE];


/*******************************************************************************
 * Private Functions
 *
 ******************************************************************************/

/* thread's stack */
char threadA_stack[KERNEL_CONF_STACKSIZE_MAIN];

/* thread's function */
void *threadA_func(void *arg);

/* main */
int main() {
	/* Initial MBoard-1 system */
	MB1_system_init();
	/* Initial BLE interface */
	ble_init();

	printf("\n************ RIOT, C++ and MBoard-1 demo program ***********\n");
	printf("\n");


	printf("}\n");

//	while(BTFlags.ready != 0x01){
//
//	}
//	ble_cmd_attributes_write(0x000b, 0, 31, msgBuf);
	while(1){

		if(newKey == MB1_usrBtn0.pressedKey_get()){
			MB1_Led_green.toggle();	//DEBUG
			/* Set BLE device discoveryable */
			ble_cmd_gap_set_mode(gap_general_discoverable, gap_undirected_connectable);
		}

		if(newKey == MB1_usrBtn1.pressedKey_get()){
			ble_cmd_attributes_write(0x000b, 0, 31, msgBuf);
		}

	}
}

/* thread A function implemetation */
void *threadA_func(void *) {
	int day = 13, month = 6, year = 2014;
	int ret_day;
	char day_of_week_table[][32] = { "Sunday", "Monday", "Tuesday", "Wednesday",
			"Thursday", "Friday", "Saturday" };

	printf("\n******** Hello, now you're in %s ********\n",
			thread_getname(thread_getpid()));
	printf("We'll test some C functions here!\n");

	printf("\n-= hello function =-\n");
	hello();

	printf("\n-= day_of_week function =-\n");

	printf("day %d, month %d, year %d is ", day, month, year);

	ret_day = day_of_week(day, month, year);
	if (ret_day >= 0) {
		printf("%s\n", day_of_week_table[ret_day]);
	}

	printf(
			"\nThis demo ends here, press Ctrl-C to exit (if you're on native)!\n");

	return NULL;
}
