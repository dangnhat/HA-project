/**
 * @file sensor_linear_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 30-10-2014
 * @brief This is header file for ADC-sensors instance having linear graph in HA system.
 */
#ifndef __HA_SENSOR_LINEAR_DRIVER_H_
#define __HA_SENSOR_LINEAR_DRIVER_H_

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

namespace sensor_linear_ns {
const uint8_t max_equation = 2;

typedef enum {
    linear = 0,
    rational = 1,
    polynomial = 2
} equation_t;

#if SND_MSG
enum
    : uint16_t {
        SEN_LINEAR_MSG
};
#endif //SND_MSG
}

class sensor_linear_instance: private adc_dev_class {
public:
    sensor_linear_instance(void);
    ~sensor_linear_instance(void);

    float (*equation[sensor_linear_ns::max_equation])(float x_value, float a,
            float b, float c);

    void device_configure(adc_config_params_t *adc_config_params);
    void set_equation(sensor_linear_ns::equation_t equation_type,
            uint8_t order_equation, float a_value, float b_value,
            float c_value);
    void set_num_equation(uint8_t num_equation);

    float get_sensor_value(void);

#if AUTO_UPDATE
    void start_sensor(void);
    void set_delta_threshold(uint16_t delta_threshold);
    void set_overflow_threshold(int overflow_threshold);
    void set_underflow_threshold(int underflow_threshold);
    float sensor_linear_processing(void);

    bool is_underlow_or_overflow(void);
#endif //AUTO_UPDATE

#if SND_MSG
    /**
     *
     */
    kernel_pid_t get_pid(void);

    /**/
    bool is_first_send = true;

    double average_value = 0.0;

    uint8_t average_num = 0;
#endif //SND_MSG
private:
    float get_voltage_value(void);
    void init_equation_table(void);
    float a_value[sensor_linear_ns::max_equation];
    float b_value[sensor_linear_ns::max_equation];
    float c_value[sensor_linear_ns::max_equation];
    uint8_t num_equation = 1;
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
#endif //SND_MSG
};

#if AUTO_UPDATE
void sensor_linear_callback_timer_isr(void);
#endif //AUTO_UPDATE

float linear_equation_calculate(float x_value, float a, float b, float c);

float rational_equation_calculate(float x_value, float a, float b, float c);

float polynomial_equation_calculate(float x_value, float a, float b, float c);

#endif //__HA_SENSOR_LINEAR_DRIVER_H_
