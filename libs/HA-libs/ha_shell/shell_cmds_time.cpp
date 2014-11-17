/**
 * @file shell_cmds_time.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 29-Oct-2014
 * @brief Source files for time/date related shell commands.
 *  Based on MB1_rtc object.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "shell_cmds_time.h"
#include "MB1_System.h"

const char date_usage[] = "Usage:\n"
        "date, show current system time.\n"
        "date mm dd yyyy hh mm ss, set system time to give time.\n";

const char dayows[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char date_line[] = "%s, %u-%u-%u %02u:%02u:%02u\n";

void date(int argc, char** argv)
{
    rtc_ns::time_t time;

    if (argc == 1) {
        MB1_rtc.get_time(time);
        printf(date_line,
                dayows[time.dayow],
                time.month,
                time.day,
                time.year,
                time.hour,
                time.min,
                time.sec);
    }
    else if (argc == 7) {
        time.month = (uint8_t)atoi(argv[1]);
        time.day = (uint8_t)atoi(argv[2]);
        time.year = (uint16_t)atoi(argv[3]);
        time.hour = (uint8_t)atoi(argv[4]);
        time.min = (uint8_t)atoi(argv[5]);
        time.sec = (uint8_t)atoi(argv[6]);

        MB1_rtc.set_time(time);
    }
    else {
        printf("%s", date_usage);
    }
}
