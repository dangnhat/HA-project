/**
 * @file ha_shell.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 27-Oct-2014
 * @brief Implementation for home automation shell.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RIOT includes */
extern "C" {
#include "posix_io.h"
#include "board_uart0.h"
#include "kernel.h"
#include "thread.h"
}

#include "ha_shell.h"

/*********************** Config interface *************************************/
namespace ha_ns {
const shell_command_t shell_commands[] = {
    /* FAT FS cmds */
    /* {"mount", "Mount FAT FS", mount}, HA_system_init will mount FAT FS */
    /* {"umount", "Unmount FAT FS", umount}, */
    {"ls", "List directory contents", ls},
    {"cat", "Concatenate files and print on the standard output", cat},
    {"touch", "Change file timestamp", touch},
    {"rm", "Remove files or directories", rm},
    {"mkdir", "Make directories", mkdir},
    {"cd", "Change working directory", cd},
    {"pwd", "Print name of current/working directory", pwd},
    {"mv", "Rename file/folder", mv},

    /* time cmds */
    {"date", "Print or set the system date and time", date},

    /* sixlowpan cmds */
    {"6lowpan", "6LoWPAN network stack configurations", sixlowpan_config},

#ifdef HA_HOST
    /* device configuration cmds */
    {"rst", "Run the specified thread with a device", rst_endpoint_callback},
    {"stop", "Stop device in the specified thread", stop_endpoint_callback},
    {"btn", "Configure button device", button_config},
    {"sw", "Configure switch device", switch_config},
    {"onoff", "Configure on-off output device", on_off_output_config},
    {"senevt", "Configure sensor event device", event_sensor_config},
    {"dimmer", "Configure dimmer device", dimmer_config},
    {"lvbulb", "Configure level bulb device", level_bulb_config},
    {"servo", "Configure servo device", servo_config},
    {"rgb", "Configure RGB-led device", rgb_led_config},
    {"senadc", "Configure ADC linear sensor device", adc_sensor_config},
#endif

#ifdef HA_CC
    /* Device list command */
    {"lsdev", "List all devices and endpoint connected to CC", controller_list_devices},
    {"scene", "Scene configuration", controller_scene_cmd},
    {"zone", "Zone configuration", controller_zone_cmd},
#endif
    {NULL, NULL, NULL}
};

/* Shell thread */
const uint16_t shell_stack_size = 2048;
char shell_stack[shell_stack_size];
const char shell_prio = PRIORITY_MAIN;
kernel_pid_t shell_pid;
}

using namespace ha_ns;

/*------------------- Static functions prototypes ----------------------------*/
static int shell_readc(void);
static void shell_putc(int c);

/*------------------- Functions ----------------------------------------------*/
void ha_shell_create(void)
{
    shell_pid = thread_create(shell_stack, shell_stack_size, shell_prio, CREATE_STACKTEST,
            ha_shell_irun, NULL, "home_automation_shell");
}

/*----------------------------------------------------------------------------*/
void* ha_shell_irun(void *arg)
{
    shell_t shell;

    (void) posix_open(uart0_handler_pid, 0);
    shell_init(&shell, shell_commands, UART0_BUFSIZE, shell_readc, shell_putc);
    shell_run(&shell);
}

/*------------------- Static functions ---------------------------------------*/
static int shell_readc(void)
{
    char c = 0;
    (void) posix_read(uart0_handler_pid, &c, 1);
    return c;
}

/*----------------------------------------------------------------------------*/
static void shell_putc(int c)
{
    (void) putchar(c);
    fflush(stdout);
}
