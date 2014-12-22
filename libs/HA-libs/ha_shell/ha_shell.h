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
#include "shell_cmds_sixlowpan.h"

#ifdef HA_HOST
#include "shell_cmds_dev_config.h"
#endif

#ifdef HA_CC
#include "controller.h"
#endif

namespace ha_ns {
extern kernel_pid_t shell_pid;
}

/*------------------- Functions ----------------------------------------------*/
/**
 * @brief   Create a thread and run ha_shell.
 *
 * @details Thread for shell will have following configurations:
 *          - shell_stack_size = 2560,
 *          - shell_prio = PRIORITY_MAIN
 *          - name = Home automation shell
 *          This shell will be based on RIOT's shell, posix_read on uart0
 *          (STM32's USART1).
 */
void ha_shell_create(void);

/**
 * @brief   Init and run the shell.
 *
 * @details This shell will be based on RIOT's shell, posix_read on uart0
 * (STM32's USART1).
 * This function will NEVER return.
 */
void* ha_shell_irun(void *arg);

#endif /* SHELL_CMDS_H_ */
