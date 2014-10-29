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
}

#include "ha_shell.h"

/*------------------- Static functions prototypes ----------------------------*/
static int shell_readc(void);
static void shell_putc(int c);

/*------------------- Functions ----------------------------------------------*/
void ha_shell_irun(void)
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
