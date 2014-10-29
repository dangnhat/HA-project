#include "integer.h"
#include "fattime.h"
#include "MB1_System.h"

DWORD get_fattime (void)
{
	DWORD res = 0;
	rtc_ns::time_t time;

	MB1_rtc.get_time(time);

	res = ((DWORD) time.year - 1980) << 25;
	res = res | (((DWORD) time.month) << 21);
	res = res | (((DWORD) time.day) << 16);
	res = res | (((DWORD) time.hour) << 11);
	res = res | (((DWORD) time.min) << 5);
	res = res | ((DWORD) time.sec)/2;

	return res;
}

