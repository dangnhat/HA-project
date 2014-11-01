/**
 * @file dimmer_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for dimmer device instance for HA system.
 */
#include "dimmer_driver.h"

#if AUTO_UPDATE
/* configurable variables */
const static uint8_t max_dimmer_dev = 16; //max num of dimmers = 16
const static uint8_t delta_threshold = 2; //delta = 2%;
const static uint16_t dimmer_sampling_time_cycle = 100; //sampling every 100ms

/* internal variables */
#if SND_MSG
static kernel_pid_t pid_table[max_dimmer_dev];
#endif
static bool table_init = false;
static dimmer_instance* dimmer_table[max_dimmer_dev];
static uint16_t time_cycle_count = 0;

/* internal function */
static void dimmer_table_init(void);
#endif

dimmer_instance::dimmer_instance(void)
{
#if AUTO_UPDATE
    this->is_over = false;
    this->old_value = 0;
#endif
}

dimmer_instance::~dimmer_instance()
{
#if AUTO_UPDATE
    this->remove_dimmer();
#endif
}

void dimmer_instance::device_configure(adc_config_params_t *adc_config_params)
{
    adc_dev_configure(adc_config_params->device_port,
            adc_config_params->device_pin, adc_config_params->adc_x,
            adc_config_params->adc_channel);
#if AUTO_UPDATE
    if (!table_init) {
        table_init = true;
        dimmer_table_init();
    }
    this->assign_dimmer();
#endif
}

uint8_t dimmer_instance::get_percent(void)
{
    uint16_t adc_value;

    adc_value = adc_dev_get_value();

    return adc_value * 100 / adc_value_max;
}

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
        this->is_over = false;
        if (delta_percent >= delta_threshold) {
            this->is_over = true;
            this->old_value = new_value_1;
        }
    } // end if()

    return new_value_1;
}

bool dimmer_instance::is_over_delta_thres(void)
{
    return this->is_over;
}

static void dimmer_table_init(void)
{
    for (int i = 0; i < max_dimmer_dev; i++) {
        dimmer_table[i] = NULL;
    }
}

void dimmer_instance::assign_dimmer(void)
{
    for (int i = 0; i < max_dimmer_dev; i++) {
        if (dimmer_table[i] == NULL) {
            dimmer_table[i] = this;
            pid_table[i] = thread_getpid();
            return;
        }
    }
}

void dimmer_instance::remove_dimmer(void)
{
    for (int i = 0; i < max_dimmer_dev; i++) {
        if (dimmer_table[i] == this) {
            dimmer_table[i] = NULL;
            pid_table[i] = KERNEL_PID_UNDEF; //redundant
            return;
        }
    }
}

void dimmer_callback_timer_isr(void)
{
    time_cycle_count = (time_cycle_count + 1) % dimmer_sampling_time_cycle;

    if (time_cycle_count == (dimmer_sampling_time_cycle - 1)) {
        for (int i = 0; i < max_dimmer_dev; i++) {
            if (dimmer_table[i] != NULL) {
                uint8_t new_value = dimmer_table[i]->dimmer_processing();
#if SND_MSG
                if (dimmer_table[i]->is_over_delta_thres()) {
                    msg_t msg;
                    msg.type = DIM_MSG;
                    msg.content.value = new_value;
                    msg_send(&msg, pid_table[i], false);
                }
#endif
            }
        }
    }
}
#endif
