/**
 * @file ha_shell.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 27-Oct-2014
 * @brief This is the entry point of shell for HA project.
 */

#ifndef HA_SHELL_H_
#define HA_SHELL_H_

#include <stddef.h>

/* RIOT's includes */
extern "C" {
#include "shell.h"
}

/* Includes other shell commands modules */
#include "shell_cmds_fatfs.h"
#include "shell_cmds_time.h"

/*********************** Config interface *************************************/
const shell_command_t shell_commands[] = {
    /* FAT FS cmds */
    {"mount", "Mount FAT FS", mount},
    {"umount", "Unmount FAT FS", umount},
    {"ls", "List directory contents", ls},
    {"cat", "Concatenate files and print on the standard output", cat},
    {"touch", "Change file timestamp", touch},
    {"rm", "Remove files or directories", rm},
    {"mkdir", "Make directories", mkdir},
    {"cd", "Change working directory", cd},
    {"pwd", "Print name of current/working directory", pwd},

    /* time cmds */
    {"date", "Print or set the system date and time", date},
    {NULL, NULL, NULL}
};

/*------------------- Functions ----------------------------------------------*/
/**
 * @brief   Init and run the shell.
 *
 * @details This shell will be based on RIOT's shell, posix_read on uart0
 * (STM32's USART1).
 * This function will NEVER return.
 */
void ha_shell_irun(void);

#endif /* SHELL_CMDS_H_ */
