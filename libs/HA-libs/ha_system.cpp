/**
 * @file ha_system.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 30-Oct-2014
 * @brief This is implement the init function for HA System.
 */

#include "ha_system.h"

#include "cc110x_reconfig.h" /* to re-init CC1101 module to use with 433MHz */
#include "diskio.h" /* for FAT FS initialization */

/******************** Config interface ****************************************/

/* 10ms or 1ms timer is required for FAT FS */
const ISRMgr_ns::ISR_t timer_1ms = ISRMgr_ns::ISRMgr_TIM6;

/*------------------- Functions ----------------------------------------------*/
void ha_system_init(netdev_t netdev) {

    /* Init MB1_system */
    MB1_system_init();

    /* Reinit CC1101 module */
    cc110x_reconfig();

    /* FAT file system module */
    MB1_ISRs.subISR_assign(timer_1ms, disk_timerproc_1ms);
}
