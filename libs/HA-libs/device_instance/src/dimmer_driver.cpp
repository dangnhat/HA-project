/**
 * @file dimmer_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for dimmer device instance for HA system.
 */
#include <string.h>
#include "dimmer_driver.h"
#if AUTO_UPDATE
#include "ha_host_glb.h"
#endif

using namespace dimmer_ns;

#if AUTO_UPDATE
/* configurable variables */
const static uint8_t delta_threshold = 3; //delta = 3%;
const static uint8_t timer_period = 1; //ms
const static uint16_t dimmer_sampling_time_cycle = 100 / timer_period; //sampling every 100ms (tim6_period = 1ms)

/* internal variables */
static bool table_init = false;
static dimmer_instance* dimmer_table[ha_host_ns::max_end_point];
static uint16_t time_cycle_count = 0;

/* internal function */
static void dimmer_table_init(void);
#endif //AUTO_UPDATE

dimmer_instance::dimmer_instance(void)
{
#if AUTO_UPDATE
    this->is_over_delta = false;
    this->old_value = 0;
    if (!table_init) {
        table_init = true;
        dimmer_table_init();
    }
#endif //AUTO_UPDATE

#if SND_MSG
    this->thread_pid = thread_getpid();
#endif //SND_MSG
}

dimmer_instance::~dimmer_instance()
{
#if AUTO_UPDATE
    this->remove_dimmer();
#endif //AUTO_UPDATE
}

void dimmer_instance::device_configure(adc_config_params_t *adc_config_params)
{
    adc_dev_configure(adc_config_params->device_port,
            adc_config_params->device_pin, adc_config_params->adc_x,
            adc_config_params->adc_channel);

    memcpy(&adc_params, adc_config_params, sizeof(adc_config_params));
#if AUTO_UPDATE
    this->assign_dimmer();
#endif //AUTO_UPDATE
}

uint8_t dimmer_instance::get_percent(void)
{
    uint16_t adc_value;

    adc_dev_configure(adc_params.device_port,
                adc_params.device_pin, adc_params.adc_x,
                adc_params.adc_channel);

    adc_value = adc_dev_get_value();

    return adc_value * 100 / adc_value_max;
}

#if SND_MSG
kernel_pid_t dimmer_instance::get_pid(void)
{
    return this->thread_pid;
}
#endif //SND_MSG

#if AUTO_UPDATE
uint8_t dimmer_instance::dimmer_processing(void)
{
    /* sampling */
    new_value_3 = new_value_2;
    new_value_2 = new_value_1;
    new_value_1 = get_percent();

    /* processing */
    if ((new_value_1 == new_value_2) && (new_value_2 == new_value_3)) { //new stable value
        uint8_t delta_percent;
        if (new_value_1 > this->old_value) {
            delta_percent = new_value_1 - this->old_value;
        } else {
            delta_percent = this->old_value - new_value_1;
        }
        this->is_over_delta = false;
        if (delta_percent >= delta_threshold) {
            this->is_over_delta = true;
            this->old_value = new_value_1;
        }
    } // end if()

    return new_value_1;
}

bool dimmer_instance::is_over_delta_thres(void)
{
    return this->is_over_delta;
}

static void dimmer_table_init(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        dimmer_table[i] = NULL;
    }
}

void dimmer_instance::assign_dimmer(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (dimmer_table[i] == NULL) {
            dimmer_table[i] = this;
            return;
        }
    }
}

void dimmer_instance::remove_dimmer(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (dimmer_table[i] == this) {
            dimmer_table[i] = NULL;
            return;
        }
    }
}

void dimmer_callback_timer_isr(void)
{
    time_cycle_count = time_cycle_count + 1;

    if (time_cycle_count == dimmer_sampling_time_cycle) {
        time_cycle_count = 0;
        for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
            if (dimmer_table[i] != NULL) {
                uint8_t new_value = dimmer_table[i]->dimmer_processing();
#if SND_MSG
                if (dimmer_table[i]->is_over_delta_thres() || dimmer_table[i]->is_first_send) {
                    dimmer_table[i]->is_first_send = false;
                    msg_t msg;
                    msg.type = DIMMER_MSG;
                    msg.content.value = new_value;
                    kernel_pid_t pid = dimmer_table[i]->get_pid();
                    if (pid == KERNEL_PID_UNDEF) {
                        return;
                    }
                    msg_send(&msg, pid, false);
                }
#endif //SND_MSG
            }
        }
    }
}
#endif //AUTO_UPDATE
