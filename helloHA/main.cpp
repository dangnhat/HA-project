#include <cstdio>
#include "MB1_System.h"

int main (void){
	
	MB1_system_init ();

	while (1){
		MB1_RGB_red.toggle ();
		delay_ms (1000);
	}
	
	return 0;
}
