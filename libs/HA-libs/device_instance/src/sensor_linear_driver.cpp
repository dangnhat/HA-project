/**
 * @file sensor_linear_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 30-10-2014
 * @brief This is source file for ADC-sensors instance having linear graph in HA system.
 */
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "sensor_linear_driver.h"

#if AUTO_UPDATE
#include "ha_node_glb.h"
#endif

#if AUTO_UPDATE
const static uint8_t timer_period = 1; //1ms
const static uint16_t sampling_time_cycle = 100 / timer_period; //sampling every 100ms (tim6_period = 1ms)

/* internal variables */
#if SND_MSG
const uint32_t send_msg_time_period = 10 * 1000 / sampling_time_cycle; //send msg every 10s.
uint16_t send_msg_time_count = 0;
#endif //SND_MSG
sensor_linear_instance* sensor_linear_table[ha_node_ns::max_end_point];
static bool table_init = false;
static uint16_t time_cycle_count = 0;

/* internal function */
static void sensor_linear_table_init(void);
#endif //AUTO_UPDATE

using namespace sensor_linear_ns;

sensor_linear_instance::sensor_linear_instance(void)
{
    init_equation_table();

#if AUTO_UPDATE
    this->is_under_or_overflow = false;
    if (!table_init) {
        table_init = true;
        sensor_linear_table_init();
    }
#endif //AUTO_UPDATE

#if SND_MSG
    this->thread_pid = thread_getpid();
#endif //SND_MSG
}

sensor_linear_instance::~sensor_linear_instance(void)
{
#if AUTO_UPDATE
    this->remove_sensor();
#endif //AUTO_UPDATE
}

void sensor_linear_instance::init_equation_table(void)
{
    for (uint8_t i = 0; i < max_equation; i++) {
        this->equation[i] = NULL;
        a_value[i] = 0.0f;
        b_value[i] = 0.0f;
        c_value[i] = 0.0f;
    }
}

void sensor_linear_instance::device_configure(
        adc_config_params_t *adc_config_params)
{
    adc_dev_configure(adc_config_params->device_port,
            adc_config_params->device_pin, adc_config_params->adc_x,
            adc_config_params->adc_channel);

    memcpy(&adc_params, adc_config_params, sizeof(adc_config_params));
}

void sensor_linear_instance::set_equation(equation_t equation_type,
        uint8_t order_equation, float a_value, float b_value, float c_value)
{
    if (order_equation > max_equation) {
        return;
    }
    switch (equation_type) {
    case rational:
        this->equation[order_equation - 1] = &rational_equation_calculate;
        break;
    case linear:
        this->equation[order_equation - 1] = &linear_equation_calculate;
        break;
    case polynomial:
        this->equation[order_equation - 1] = &polynomial_equation_calculate;
        break;
    default:
        return;
    }

    this->a_value[order_equation - 1] = a_value;
    this->b_value[order_equation - 1] = b_value;
    this->c_value[order_equation - 1] = c_value;
}

void sensor_linear_instance::set_num_equation(uint8_t num_equation)
{
    if (num_equation > max_equation) {
        num_equation = max_equation;
    }
    this->num_equation = num_equation;
}

float sensor_linear_instance::get_voltage_value(void)
{
    /* reconfigure */
    adc_dev_configure(adc_params.device_port, adc_params.device_pin,
            adc_params.adc_x, adc_params.adc_channel);

    float converted_adc = adc_dev_get_value();
    float converted_volt = converted_adc * ((float) v_ref)
            / ((float) adc_value_max); //mV

    return converted_volt / 1000.0f; //V
}

float sensor_linear_instance::get_sensor_value(void)
{
    float sensor_value = get_voltage_value();

    for (uint8_t i = 0; i < this->num_equation; i++) {
        if (this->equation[i] != NULL) {
            sensor_value = this->equation[i](sensor_value, this->a_value[i],
                    this->b_value[i], this->c_value[i]);
        }
    }

    return sensor_value;
}

float linear_equation_calculate(float x_value, float a_value, float b_value,
        float c_value)
{
    /* y = a*x + b */
    return x_value * a_value + b_value;
}

float rational_equation_calculate(float x_value, float a_value, float b_value,
        float c_value)
{
    /* y = 1/(a*x +b) + c*/
    return 1.0f / (x_value * a_value + b_value) + c_value;
}

float polynomial_equation_calculate(float x_value, float a_value, float b_value,
        float c_value)
{
    return a_value * pow(x_value, b_value) + c_value;
}

#if AUTO_UPDATE
void sensor_linear_instance::start_sensor(void)
{
    this->assign_sensor();
}

float sensor_linear_instance::sensor_linear_processing(void)
{
    float new_value = get_sensor_value();
    uint16_t delta_value;

    if (new_value > old_sensor_value) {
        delta_value = new_value - old_sensor_value;
    } else {
        delta_value = old_sensor_value - new_value;
    }

    is_under_or_overflow = false;
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

void sensor_linear_instance::set_overflow_threshold(int overflow_threshold)
{
    this->overflow_thres = overflow_threshold;
}

void sensor_linear_instance::set_underflow_threshold(int underflow_threshold)
{
    this->underflow_thres = underflow_threshold;
}

bool sensor_linear_instance::is_underlow_or_overflow(void)
{
    return this->is_under_or_overflow;
}

void sensor_linear_instance::assign_sensor(void)
{
    for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        if (sensor_linear_table[i] == NULL) {
            sensor_linear_table[i] = this;
            return;
        }
    }
}

void sensor_linear_instance::remove_sensor(void)
{
    for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        if (sensor_linear_table[i] == this) {
            sensor_linear_table[i] = NULL;
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
#endif //SND_MSG
        for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
            if (sensor_linear_table[i] != NULL) {
                sensor_linear_table[i]->average_value +=
                        sensor_linear_table[i]->sensor_linear_processing();
                sensor_linear_table[i]->average_num++;
#if SND_MSG
                if (sensor_linear_table[i]->is_underlow_or_overflow()
                        || send_msg_time_count
                                == (send_msg_time_period - sampling_time_cycle)
                        || sensor_linear_table[i]->is_first_send) {
                    sensor_linear_table[i]->is_first_send = false;
                    msg_t msg;
                    msg.type = SEN_LINEAR_MSG;
                    msg.content.value = (uint16_t) round(
                            sensor_linear_table[i]->average_value
                                    / sensor_linear_table[i]->average_num);
                    sensor_linear_table[i]->average_value = 0;
                    sensor_linear_table[i]->average_num = 0;
                    kernel_pid_t pid = sensor_linear_table[i]->get_pid();
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

static void sensor_linear_table_init(void)
{
    for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        sensor_linear_table[i] = NULL;
    }
}
#endif //AUTO_UPDATE

#if SND_MSG
kernel_pid_t sensor_linear_instance::get_pid(void)
{
    return this->thread_pid;
}
#endif //SND_MSG
