/**
 * @file ha_system.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 30-Oct-2014
 * @brief This is the entry point of HA System.
 *
 * (FAT File system)
 * - Using MB1_rtc object to get system time.
 * - Assign 1 interrupt handler to ISR_TIM6
 *
 * (Transceiver)
 * - RIOT's auto_init module will start transceiver when we use net_if module.
 * (CC1101 will be init-ed for 915MHz).
 * - CC1101 will be re-init-ed to 433MHz.
 *
 */

#ifndef HA_SYSTEM_H_
#define HA_SYSTEM_H_

/* RIOT's includes */
extern "C" {
#include "thread.h"
#include "vtimer.h"
#include "msg.h"
}

/* MBoard-1 includes */
#include "MB1_System.h"

/* HA libs includes */
#include "ha_shell.h"
#include "ff.h"

/* Typedefs */
typedef enum: uint8_t {
    node = 0,
    cc = 1,
} netdev_t;

/*------------------- Functions ----------------------------------------------*/
/**
 * @brief   Init Home automation system.
 *
 * @details
 *
 * @param[in]   netdev, type of device in the HA network. This could be node or cc.
 */
void ha_system_init(netdev_t netdev);

#endif /* HA_SYSTEM_H_ */
