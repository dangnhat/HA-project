/**
 * @file ha_node.h
 * @author  Nguyen Van Hien  <nvhien1992@gmail.com>.
 * @version 1.0
 * @date 11-Nov-2014
 * @brief This is source file for HA node initialization in HA system.
 *
 * (Pid table)
 * Initialize endpoint pid table.
 *
 * (Timer6)
 * Assign callbacks into interrupt of tim6.
 */
extern "C" {
#include "msg.h"
#include "thread.h"
}
#include "ha_node.h"
#include "MB1_System.h"

const ISRMgr_ns::ISR_t tim_isr_type = ISRMgr_ns::ISRMgr_TIM6;
const uint8_t timer_period = 1; //ms
const uint32_t send_alive_time_period = 60 * 1000 / timer_period; //send alive every 60s.

uint32_t time_cycle_count = 0;
kernel_pid_t ha_node_ns::end_point_pid[max_end_point];

static void endpoint_pid_table_init(void);
static void send_alive_timer_isr(void);

void ha_node_init(void)
{
    endpoint_pid_table_init();

    /* Assign send-alive callback function into interrupt timer */
    MB1_ISRs.subISR_assign(tim_isr_type, &send_alive_timer_isr);

    /* Assign button & switch callback function into interrupt timer */
    MB1_ISRs.subISR_assign(tim_isr_type, &btn_sw_callback_timer_isr);

    /* Assign dimmer callback function into interrupt timer */
    MB1_ISRs.subISR_assign(tim_isr_type, &dimmer_callback_timer_isr);

    /* Assign ADC linear-sensor callback function into interrupt timer */
    MB1_ISRs.subISR_assign(tim_isr_type, &sensor_linear_callback_timer_isr);
}

static void endpoint_pid_table_init(void)
{
    for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        ha_node_ns::end_point_pid[i] = KERNEL_PID_UNDEF;
    }
}

static void send_alive_timer_isr(void)
{
    time_cycle_count = time_cycle_count + 1;
    if (time_cycle_count == send_alive_time_period) {
        time_cycle_count = 0;
        for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
            if (ha_node_ns::end_point_pid[i] != KERNEL_PID_UNDEF) {
                msg_t msg;
                msg.type = ha_node_ns::SEND_ALIVE;
                msg_send(&msg, ha_node_ns::end_point_pid[i], false);
            }
        }
    }
}
