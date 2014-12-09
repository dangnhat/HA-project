/**
 * @file adc_sensor_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 30-10-2014
 * @brief This is header file for ADC-sensors in HA system.
 */
#ifndef __HA_ADC_SENSOR_DRIVER_H_
#define __HA_ADC_SENSOR_DRIVER_H_

/* using interrupt timer to update value automatically */
#define AUTO_UPDATE (1)

#if AUTO_UPDATE

/* send updated value to its thread */
#define SND_MSG (1)
#if SND_MSG
extern "C" {
#include "msg.h"
#include "thread.h"
}
#endif //SND_MSG
#endif //AUTO_UPDATE

#include "ADC_device.h"

namespace adc_sensor_ns {
const uint8_t max_equation = 2;

typedef enum {
    linear = 0,
    rational = 1,
    polynomial = 2
} equation_t;

#if SND_MSG
enum
    : uint16_t {
        ADC_SENSOR_MSG
};
#endif //SND_MSG
}

class adc_sensor_instance: private adc_dev_class {
public:
    adc_sensor_instance(void);
    ~adc_sensor_instance(void);

    void device_configure(adc_config_params_t *adc_config_params);

    void set_equation_type(char* equation_type_buff, uint8_t buff_size);
    void set_equation_params(float* equation_params_buff, uint8_t buff_size);
    float cal_iterative_equations(float first_value);
    float get_sensor_value(void);

#if AUTO_UPDATE
    void start_sensor(void);
    void set_delta_threshold(uint16_t delta_threshold);
    void set_overflow_threshold(int overflow_threshold);
    void set_underflow_threshold(int underflow_threshold);
    float adc_sensor_processing(void);

    bool is_underlow_or_overflow(void);
#endif //AUTO_UPDATE

#if SND_MSG
    /**
     *
     */
    kernel_pid_t get_pid(void);
#endif //SND_MSG
private:
    float get_voltage_value(void);
    float* equation_params_buffer = NULL;
    float* param_ptr = NULL;
    uint8_t param_index = 0;
    char* equation_type_buffer = NULL;
    uint8_t num_equation = 0;
    uint8_t num_params = 0;
    adc_config_params_t adc_params;

#if AUTO_UPDATE
    bool is_under_or_overflow;
    uint16_t delta_thres = 0;
    int overflow_thres = 0;
    int underflow_thres = 0;

    float old_sensor_value = 0;

    void assign_sensor(void);
    void remove_sensor(void);
#endif //AUTO_UPDATE

#if SND_MSG
    kernel_pid_t thread_pid;

    /**/
    bool is_first_send = true;

    double total_value = 0.0;

    uint16_t average_num = 0;
#endif //SND_MSG
};

#if AUTO_UPDATE
void adc_sensor_callback_timer_isr(void);
#endif //AUTO_UPDATE

float linear_equation_calculate(float x_value, float a, float b); //y = ax+b;

float rational_equation_calculate(float x_value, float a, float b, float c); //y = 1/(ax+b)+c;

float polynomial_equation_calculate(float x_value, float a, float b, float c); //y = ax^b+c;

float lookup_in_table(float value, float* defined_table, uint8_t table_size);

#endif //__HA_ADC_SENSOR_DRIVER_H_
