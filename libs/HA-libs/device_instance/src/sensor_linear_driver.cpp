/**
 * @file sensor_linear_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 30-10-2014
 * @brief This is source file for ADC-sensors instance having linear graph in HA system.
 */
#include "sensor_linear_driver.h"
#include <stdio.h>
#if AUTO_UPDATE
const static uint8_t max_sensor_linear = 16;
const static uint8_t timer_period = 1; //1ms
const static uint16_t sampling_time_cycle = 100 / timer_period; //sampling every 100ms (tim6_period = 1ms)

/* internal variables */
#if SND_MSG
const static uint32_t send_msg_time_period = 30 * 1000 / sampling_time_cycle; //send msg every 30s.
static uint16_t send_msg_time_count = 0;
static kernel_pid_t pid_table[max_sensor_linear];
#endif
static bool table_init = false;
static sensor_linear_instance* sensor_linear_table[max_sensor_linear];
static uint16_t time_cycle_count = 0;

/* internal function */
static void sensor_linear_table_init(void);
#endif

using namespace sensor_linear_ns;

sensor_linear_instance::sensor_linear_instance(void)
{
    /* linear equation as default */
    this->equation_type = linear;
    this->equation = &linear_equation_calculate;

#if AUTO_UPDATE
    this->is_under_or_overflow = false;
#endif
}

sensor_linear_instance::~sensor_linear_instance(void)
{
#if AUTO_UPDATE
    this->remove_sensor();
    if (!table_init) {
        table_init = true;
        sensor_linear_table_init();
    }
#endif
}

void sensor_linear_instance::device_configure(
        adc_config_params_t *adc_config_params)
{
    adc_dev_configure(adc_config_params->device_port,
            adc_config_params->device_pin, adc_config_params->adc_x,
            adc_config_params->adc_channel);
}

void sensor_linear_instance::set_equation(equation_t equation_type,
        float a_factor, float b_constant)
{
    this->equation_type = equation_type;
    if (equation_type == rational) {
        this->equation = &rational_equation_calculate;
    }

    this->a_factor = a_factor;
    this->b_constant = b_constant;
}

float sensor_linear_instance::get_voltage_value(void)
{
    float converted_volt;
    uint16_t converted_adc = adc_dev_get_value();
    converted_volt = converted_adc * v_ref / adc_value_max; //mV

    return converted_volt / 1000; //V
}

uint16_t sensor_linear_instance::get_sensor_value(void)
{
    float converted_volt = get_voltage_value();
    return this->equation(converted_volt, this->a_factor, this->b_constant);
}

float linear_equation_calculate(float x_value, float a_factor, float b_constant)
{
    /* y = a*x + b */
    return x_value * a_factor + b_constant;
}

float rational_equation_calculate(float x_value, float a_factor,
        float b_constant)
{
    /* y = 1/(a*x +b) */
    return 1 / (x_value * a_factor + b_constant);
}

#if AUTO_UPDATE
void sensor_linear_instance::start(void)
{
    this->assign_sensor();
}

uint16_t sensor_linear_instance::sensor_linear_processing(void)
{
    uint16_t new_value = get_sensor_value();
    uint16_t delta_value;

    if (new_value > old_sensor_value) {
        delta_value = new_value - old_sensor_value;
    } else {
        delta_value = old_sensor_value - new_value;
    }

    if (delta_value >= delta_thres) {
        old_sensor_value = new_value;
        if (old_sensor_value >= overflow_thres
                || old_sensor_value <= underflow_thres) {
            is_under_or_overflow = true;
        }
    }

    return new_value;
}

void sensor_linear_instance::set_delta_threshold(uint16_t delta_threshold)
{
    this->delta_thres = delta_threshold;
}

void sensor_linear_instance::set_overflow_threshold(uint16_t overflow_threshold)
{
    this->overflow_thres = overflow_threshold;
}

void sensor_linear_instance::set_underflow_threshold(
        uint16_t underflow_threshold)
{
    this->underflow_thres = underflow_threshold;
}

bool sensor_linear_instance::is_underlow_or_overflow(void)
{
    bool retval = this->is_under_or_overflow;
    this->is_under_or_overflow = false;

    return retval;
}

void sensor_linear_instance::assign_sensor(void)
{
    for (uint8_t i = 0; i < max_sensor_linear; i++) {
        if (sensor_linear_table[i] == NULL) {
            sensor_linear_table[i] = this;
#if SND_MSG
            pid_table[i] = thread_getpid();
#endif
            return;
        }
    }
}

void sensor_linear_instance::remove_sensor(void)
{
    for (uint8_t i = 0; i < max_sensor_linear; i++) {
        if (sensor_linear_table[i] == this) {
            sensor_linear_table[i] = NULL;
#if SND_MSG
            pid_table[i] = KERNEL_PID_UNDEF; //redundant
#endif
            return;
        }
    }
}

void sensor_linear_callback_timer_isr(void)
{
    time_cycle_count = time_cycle_count + 1;

    if (time_cycle_count == sampling_time_cycle) {
        time_cycle_count = 0;
#if SND_MSG
        send_msg_time_count = (send_msg_time_count + 1) % send_msg_time_period;
#endif
        for (uint8_t i = 0; i < max_sensor_linear; i++) {
            if (sensor_linear_table[i] != NULL) {
                uint16_t new_value =
                        sensor_linear_table[i]->sensor_linear_processing();
#if SND_MSG
                if (sensor_linear_table[i]->is_underlow_or_overflow()
                        || send_msg_time_count
                                == (send_msg_time_period - sampling_time_cycle)) {
                    msg_t msg;
                    msg.type = SEN_LINEAR_MSG;
                    msg.content.value = new_value;
                    msg_send(&msg, pid_table[i], false);
                }
#endif
            }
        }

    }
}

static void sensor_linear_table_init(void)
{
    for (uint8_t i = 0; i < max_sensor_linear; i++) {
        sensor_linear_table[i] = NULL;
    }
}
#endif
