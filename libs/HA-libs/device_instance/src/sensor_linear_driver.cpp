/**
 * @file sensor_linear_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 30-10-2014
 * @brief This is source file for ADC-sensors instance having linear graph in HA system.
 */
#include "sensor_linear_driver.h"
#if AUTO_UPDATE
#include "ha_node_glb.h"
#endif

#if AUTO_UPDATE
const static uint8_t timer_period = 1; //1ms
const static uint16_t sampling_time_cycle = 100 / timer_period; //sampling every 100ms (tim6_period = 1ms)

/* internal variables */
#if SND_MSG
const uint32_t send_msg_time_period = 30 * 1000 / sampling_time_cycle; //send msg every 30s.
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
    /* linear equation as default */
    this->equation = &linear_equation_calculate;

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
    if (equation_type == rational) {
        this->equation = &rational_equation_calculate;
    }

    this->a_factor = a_factor;
    this->b_constant = b_constant;
}

float sensor_linear_instance::get_voltage_value(void)
{
    float converted_adc = adc_dev_get_value();
    float converted_volt = converted_adc * ((float) v_ref)
            / ((float) adc_value_max); //mV

    return converted_volt / 1000.0f; //V
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
    return 1.0f / (x_value * a_factor + b_constant);
}

#if AUTO_UPDATE
void sensor_linear_instance::start_sensor(void)
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
        is_under_or_overflow = false;
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
                uint16_t new_value =
                        sensor_linear_table[i]->sensor_linear_processing();
#if SND_MSG
                if (sensor_linear_table[i]->is_underlow_or_overflow()
                        || send_msg_time_count
                                == (send_msg_time_period - sampling_time_cycle)) {
                    msg_t msg;
                    msg.type = SEN_LINEAR_MSG;
                    msg.content.value = new_value;
                    kernel_pid_t pid = sensor_linear_table[i]->get_pid();
                    if(pid == KERNEL_PID_UNDEF) {
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
