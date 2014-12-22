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
#include "ha_sixlowpan.h"

/******************** Config interface ****************************************/
#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

/* 10ms or 1ms timer is required for FAT FS */
const ISRMgr_ns::ISR_t timer_1ms = ISRMgr_ns::ISRMgr_TIM6;

const char default_drive_path[] = "0:/";
FATFS fatfs;

/* 6LoWPAN restart */
Button* stop_resart_slp_btn_p = &MB1_usrBtn1;

/*------------------- Functions ----------------------------------------------*/
void ha_system_init(void)
{
    FRESULT fres;

    /* Init MB1_system */
    MB1_system_init();
    HA_NOTIFY("MB1_system initialized.\n");

    /* Reinit CC1101 module. */
    cc110x_reconfig();
    HA_NOTIFY("CC1101 configured to 390MHz, 0dBm.\n");

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

    /* Start CC's 6LoWPAN threads */
    slp_sender_start();
    slp_receiver_start();

#ifdef HA_HOST
    /* Node's specific initializations */
    ha_host_init();

    HA_NOTIFY("HA node initialized\n");
#endif

#ifdef HA_CC
    /* CC's specific initializations */
    controller_start();
    MB1_ISRs.subISR_assign(ISRMgr_ns::ISRMgr_RTC, second_int_callback);

    /* Start ble thread */
    MB1_ISRs.subISR_assign(ISRMgr_ns::ISRMgr_TIM6, ble_timeout_TIM6_ISR);
    ble_thread_start();
    ble_init();
#endif

    /* Prompt and restart 6LoWPAN thread */
    ha_slp_start_on_reset(stop_resart_slp_btn_p, "UsrBtn1");
}
