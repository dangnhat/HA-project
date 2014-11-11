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
#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

/* 10ms or 1ms timer is required for FAT FS */
const ISRMgr_ns::ISR_t timer_1ms = ISRMgr_ns::ISRMgr_TIM6;

const char default_drive_path[] = "0:/";
FATFS fatfs;

/*------------------- Functions ----------------------------------------------*/
void ha_system_init(void) {
    FRESULT fres;

    /* Init MB1_system */
    MB1_system_init();
    HA_NOTIFY("MB1_system initialized.\n");

    /* Reinit CC1101 module. TODO: need to check channel, freq again */
//    cc110x_reconfig();
//    HA_NOTIFY("CC1101 configured to 433MHz.\n");

    /* FAT file system module */
    MB1_ISRs.subISR_assign(timer_1ms, disk_timerproc_1ms);

    fres = f_mount(&fatfs, default_drive_path, 1);
    if (fres != FR_OK) {
        print_ferr(fres);
        HA_NOTIFY("FAT FS is NOT mounted.\n");
    }
    else {
        HA_NOTIFY("FAT FS is mounted to %s\n", default_drive_path);
    }


#ifdef HA_NODE
    /* Node's specific initializations */
    /* Button & switch callback function */
    MB1_ISRs.subISR_assign(timer_1ms, &btn_sw_callback_timer_isr);

    /* Dimmer callback function */
    MB1_ISRs.subISR_assign(timer_1ms, &dimmer_callback_timer_isr);
#endif

#ifdef HA_CC
    /* CC's specific initializations */
    /* Start CC's 6LoWPAN threads */
    cc_slp_sender_start();
    cc_slp_receiver_start();
#endif

    /* Create shell thread */
    ha_shell_create();
    HA_NOTIFY("Home Automation shell started.\n");
}
