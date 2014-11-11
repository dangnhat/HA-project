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

    /* time cmds */
    {"date", "Print or set the system date and time", date},

    /* sixlowpan cmds */
    {"6lowpan", "6LoWPAN network stack configurations", sixlowpan_config},

    {NULL, NULL, NULL}
};

/* Shell thread */
const uint16_t shell_stack_size = 2560;
char shell_stack[shell_stack_size];
const char shell_prio = PRIORITY_MAIN;
kernel_pid_t shell_pid;
}

using namespace ha_ns;

/*------------------- Static functions prototypes ----------------------------*/
static int shell_readc(void);
static void shell_putc(int c);
static void* shell_irun(void *);

/*------------------- Functions ----------------------------------------------*/
void ha_shell_create(void)
{
    shell_pid = thread_create(shell_stack, shell_stack_size, shell_prio, CREATE_STACKTEST,
            shell_irun, NULL, "home_automation_shell");
}

/*------------------- Static functions ---------------------------------------*/
/**
 * @brief   Init and run the shell.
 *
 * @details This shell will be based on RIOT's shell, posix_read on uart0
 * (STM32's USART1).
 * This function will NEVER return.
 */
static void* shell_irun(void *)
{
    shell_t shell;

    (void) posix_open(uart0_handler_pid, 0);
    shell_init(&shell, shell_commands, UART0_BUFSIZE, shell_readc, shell_putc);
    shell_run(&shell);
}

/*----------------------------------------------------------------------------*/
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
