/**
 * @file adc_sensor_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 30-10-2014
 * @brief This is source file for ADC-sensors in HA system.
 */
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "adc_sensor_driver.h"

#if AUTO_UPDATE
#include "ha_host_glb.h"
#endif

#if AUTO_UPDATE
const static uint8_t timer_period = 1; //1ms
const static uint16_t sampling_time_cycle = 100 / timer_period; //sampling every 100ms (tim6_period = 1ms)

/* internal variables */
#if SND_MSG
const uint32_t send_msg_time_period = 15 * 1000 / sampling_time_cycle; //send msg every 15s.
uint16_t send_msg_time_count = 0;
#endif //SND_MSG
adc_sensor_instance* adc_sensor_table[ha_host_ns::max_end_point]; // the number of sensors depend on the number of EPs.
static bool table_init = false;
static uint16_t time_cycle_count = 0;

/* internal function */
static void adc_sensor_table_init(void);
#endif //AUTO_UPDATE

using namespace adc_sensor_ns;

adc_sensor_instance::adc_sensor_instance(void)
{
#if AUTO_UPDATE
    this->is_under_or_overflow = false;
    if (!table_init) {
        table_init = true;
        adc_sensor_table_init();
    }
#endif //AUTO_UPDATE

#if SND_MSG
    this->thread_pid = thread_getpid();
#endif //SND_MSG
}

adc_sensor_instance::~adc_sensor_instance(void)
{
#if AUTO_UPDATE
    this->remove_sensor();
#endif //AUTO_UPDATE
}

void adc_sensor_instance::device_configure(
        adc_config_params_t *adc_config_params)
{
    adc_dev_configure(adc_config_params->device_port,
            adc_config_params->device_pin, adc_config_params->adc_x,
            adc_config_params->adc_channel);

    memcpy(&adc_params, adc_config_params, sizeof(adc_config_params));
}

void adc_sensor_instance::set_equation_type(char* equation_type_buff,
        uint8_t buff_size)
{
    this->equation_type_buffer = equation_type_buff;
    this->num_equation = buff_size;
}

void adc_sensor_instance::set_equation_params(float* equation_params_buff,
        uint8_t buff_size)
{
    this->equation_params_buffer = equation_params_buff;
    this->num_params = buff_size;
}

float adc_sensor_instance::get_voltage_value(void)
{
    /* reconfigure */
    adc_dev_configure(adc_params.device_port, adc_params.device_pin,
            adc_params.adc_x, adc_params.adc_channel);

    float converted_adc = adc_dev_get_value();
    float converted_volt = converted_adc * ((float) v_ref)
            / ((float) adc_value_max); //mV

    return converted_volt / 1000.0f; //V
}

float adc_sensor_instance::get_sensor_value(void)
{
    float sensor_value = get_voltage_value();

    sensor_value = cal_iterative_equations(sensor_value);

    return sensor_value;
}

float adc_sensor_instance::cal_iterative_equations(float first_value)
{
    /* check parameters */
    if (!equation_type_buffer || !equation_params_buffer) {
        return 0;
    }
    uint8_t consumed_params = 0; // the number of params was consumed.
    float* param_ptr = this->equation_params_buffer;

    float retval = first_value;
    for (uint8_t i = 0; i < num_equation; i++) {
        switch (equation_type_buffer[i]) {
        case 'l': //linear
            consumed_params += 2;
            if (consumed_params > num_params) {
                return retval;
            }
            retval = linear_equation_calculate(retval, *param_ptr++,
                    *param_ptr++);
            break;
        case 'r': //rational
            consumed_params += 3;
            if (consumed_params > num_params) {
                return retval;
            }
            retval = rational_equation_calculate(retval, *param_ptr++,
                    *param_ptr++, *param_ptr++);
            break;
        case 'p': //polynomial
            consumed_params += 3;
            if (consumed_params > num_params) {
                return retval;
            }
            retval = polynomial_equation_calculate(retval, *param_ptr++,
                    *param_ptr++, *param_ptr++);
            break;
        case 't': //table
            return lookup_table(retval, param_ptr, num_params - consumed_params);
        default:
            break;
        }
    }

    return retval;
}

float linear_equation_calculate(float x_value, float a_value, float b_value)
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
    /* y = a*x^b + c */
    return a_value * pow(x_value, b_value) + c_value;
}

float lookup_table(float value, float* defined_table, uint8_t table_size)
{
    if (!defined_table || (table_size % 2 != 0)) {
        return value;
    }

    bool inc_seq = false;
    if (defined_table[0] < defined_table[table_size - 2]) {
        inc_seq = true;
    }

    float a_value = 0;
    float b_value = 0;
    /* find segment */
    uint8_t index = 0;
    while (index < table_size) {
        if ((index % 2 == 0)) {
            /* if finding out exact input value
             * or input value is greater than max x_value in table,
             * returning the y_value */
            if (inc_seq) { //increasing sequence
                if ((index == table_size - 2)
                        && (value >= defined_table[index])) {
                    return defined_table[index + 1];
                }

                /* x1 <= value <= x2 */
                if (value >= defined_table[index]
                        && value <= defined_table[index + 2]) {
                    break;
                }
            } else { //decreasing sequence
                if ((index == table_size - 2)
                        && (value <= defined_table[index])) {
                    return defined_table[index + 1];
                }

                /* x2 <= value <= x1 */
                if (value <= defined_table[index]
                        && value >= defined_table[index + 2]) {
                    break;
                }
            }
        }
        index++;
    }

    if (index == table_size - 1) {
        return defined_table[1];
    }

    /* cal the ref value by linearing input value in the found out segment */
    a_value = (defined_table[index + 3] - defined_table[index + 1])
            / (defined_table[index + 2] - defined_table[index]); //a = (y2-y1)/(x2-x1)
    b_value = (defined_table[index + 1] * defined_table[index + 2]
            - defined_table[index + 3] * defined_table[index])
            / (defined_table[index + 2] - defined_table[index]); //b = (y1*x2 - y2*x1)/(x2-x1)

    /* y = a*x + b */
    return value * a_value + b_value;
}

#if AUTO_UPDATE
void adc_sensor_instance::start_sensor(void)
{
    this->assign_sensor();
}

float adc_sensor_instance::adc_sensor_processing(void)
{
    float new_value = get_sensor_value();
    uint16_t delta_value;

#if SND_MSG
    if (is_first_send) {
        is_first_send = false;
        is_under_or_overflow = true;
        return old_sensor_value = new_value;
    }
#endif //SND_MSG

    if (new_value > old_sensor_value) {
        delta_value = new_value - old_sensor_value;
    } else {
        delta_value = old_sensor_value - new_value;
    }

    /* calculate average value */
    total_value += new_value;
    average_num++;
    if (average_num == 10) {
        old_sensor_value = total_value / ((float) average_num);
        average_num = 0;
        total_value = 0;
    }

    is_under_or_overflow = false;
    if (delta_value >= delta_thres) {
        if (new_value >= overflow_thres || new_value <= underflow_thres) {
            is_under_or_overflow = true;
            old_sensor_value = new_value;
        }
    }

    return old_sensor_value;
}

void adc_sensor_instance::set_delta_threshold(uint16_t delta_threshold)
{
    this->delta_thres = delta_threshold;
}

void adc_sensor_instance::set_overflow_threshold(int overflow_threshold)
{
    this->overflow_thres = overflow_threshold;
}

void adc_sensor_instance::set_underflow_threshold(int underflow_threshold)
{
    this->underflow_thres = underflow_threshold;
}

bool adc_sensor_instance::is_underlow_or_overflow(void)
{
    return this->is_under_or_overflow;
}

void adc_sensor_instance::assign_sensor(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (adc_sensor_table[i] == NULL) {
            adc_sensor_table[i] = this;
            return;
        }
    }
}

void adc_sensor_instance::remove_sensor(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (adc_sensor_table[i] == this) {
            adc_sensor_table[i] = NULL;
            return;
        }
    }
}

void adc_sensor_callback_timer_isr(void)
{
    time_cycle_count = time_cycle_count + 1;

    if (time_cycle_count == sampling_time_cycle) {
        time_cycle_count = 0;
#if SND_MSG
        send_msg_time_count = (send_msg_time_count + 1) % send_msg_time_period;
#endif //SND_MSG
        for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
            if (adc_sensor_table[i] != NULL) {
                float ss_value = adc_sensor_table[i]->adc_sensor_processing();
#if SND_MSG
                if (adc_sensor_table[i]->is_underlow_or_overflow()
                        || send_msg_time_count
                                == (send_msg_time_period - sampling_time_cycle)) {
                    msg_t msg;
                    msg.type = ADC_SENSOR_MSG;
                    msg.content.value = (uint16_t) round(ss_value);
                    kernel_pid_t pid = adc_sensor_table[i]->get_pid();
                    if (pid == KERNEL_PID_UNDEF) {
                        return;
                    }
                    msg_send(&msg, pid, false);
                }
#endif //SND_MSG
            } //end if()
        } // end for()
    } //end if()
}

static void adc_sensor_table_init(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        adc_sensor_table[i] = NULL;
    }
}
#endif //AUTO_UPDATE

#if SND_MSG
kernel_pid_t adc_sensor_instance::get_pid(void)
{
    return this->thread_pid;
}
#endif //SND_MSG
