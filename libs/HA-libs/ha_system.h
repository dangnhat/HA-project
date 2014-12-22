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

#include <stdio.h>
#include <stdint.h>

/* MBoard-1 includes */
#include "MB1_System.h"

/* HA libs includes */
#include "ha_shell.h"
#include "ff.h"
#include "device_id.h"
#include "gff_mesg_id.h"

#ifdef HA_HOST              /* Host specific includes */
#include "ha_host.h"
#endif

#ifdef HA_CC                /* CC specific includes */
#include "controller.h"
#include "ble_transaction.h"
#endif

/* Typedefs */
namespace ha_ns {
}

/*------------------- Functions ----------------------------------------------*/
/**
 * @brief   Init Home automation system.
 *          It depends on HA_CC or HA_HOST definitions in command line.
 */
void ha_system_init(void);

#endif /* HA_SYSTEM_H_ */
