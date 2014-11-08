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

#ifdef HA_NODE
const uint8_t timer_period = 1; //ms
const uint32_t send_alive_time_period = 60 * 1000 / timer_period; //send alive every 60s.

uint32_t time_cycle_count = 0;

static void endpoint_pid_table_init(void);
void send_alive_timer_isr(void);
#endif

/*------------------- Functions ----------------------------------------------*/
void ha_system_init(void)
{
    FRESULT fres;

    /* Init MB1_system */
    MB1_system_init();
    HA_NOTIFY("MB1_system initialized.\n");

    /* Reinit CC1101 module */
    cc110x_reconfig();
    HA_NOTIFY("CC1101 configured to 433MHz.\n");

    /* FAT file system module */
    MB1_ISRs.subISR_assign(timer_1ms, disk_timerproc_1ms);

    fres = f_mount(&fatfs, default_drive_path, 1);
    if (fres != FR_OK) {
        print_ferr(fres);
    }
    HA_NOTIFY("FAT FS is mounted to %s\n", default_drive_path);

#ifdef HA_NODE
    /* Node's specific initializations */
    /* Initialize pid table of endpoints */
    endpoint_pid_table_init();

    /* Send alive callback function */
    MB1_ISRs.subISR_assign(timer_1ms, &send_alive_timer_isr);

    /* Button & switch callback function */
    MB1_ISRs.subISR_assign(timer_1ms, &btn_sw_callback_timer_isr);

    /* Dimmer callback function */
    MB1_ISRs.subISR_assign(timer_1ms, &dimmer_callback_timer_isr);

    /* Sensor-linear ADC callback function */
    MB1_ISRs.subISR_assign(timer_1ms, &sensor_linear_callback_timer_isr);
#endif

#ifdef HA_CC
    /* CC's specific initializations */
#endif
}

#ifdef HA_NODE
static void endpoint_pid_table_init(void)
{
    for (uint8_t i = 0; i < max_end_point; i++) {
        end_point_pid[i] = KERNEL_PID_UNDEF;
    }
}

void send_alive_timer_isr(void)
{
    time_cycle_count = time_cycle_count + 1;
    if (time_cycle_count == send_alive_time_period) {
        time_cycle_count = 0;
        for (uint8_t i = 0; i < max_end_point; i++) {
            if (end_point_pid[i] != KERNEL_PID_UNDEF) {
                msg_t msg;
                msg.type = ha_node_ns::SEND_ALIVE;
                msg_send(&msg, end_point_pid[i], false);
            }
        }
    }
}
#endif
