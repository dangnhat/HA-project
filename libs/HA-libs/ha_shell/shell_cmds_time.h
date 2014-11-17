/**
 * @file shell_cmds_time.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 29-Oct-2014
 * @brief Header files for time/date related shell commands.
 */

#ifndef SHELL_CMDS_TIME_H_
#define SHELL_CMDS_TIME_H_

/**
 * @brief   Get/Set system time and date.
 *
 * @details Usage:  date, show current system time.
 *                  date mm dd yyyy hh mm ss, set system time to the given time.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void date(int argc, char** argv);

#endif /* SHELL_CMDS_TIME_H_ */
