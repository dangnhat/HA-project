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
typedef enum {
    linear = 0,
    rational = 1
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

    float (*equation)(float raw_voltage_value, float a_factor, float b_constant);

    void device_configure(adc_config_params_t *adc_config_params);
    void set_equation(sensor_linear_ns::equation_t equation_type,
            float a_factor, float b_constant);

    uint16_t get_sensor_value(void);

#if AUTO_UPDATE
    void start_sensor(void);
    void set_delta_threshold(uint16_t delta_threshold);
    void set_overflow_threshold(uint16_t overflow_threshold);
    void set_underflow_threshold(uint16_t underflow_threshold);
    uint16_t sensor_linear_processing(void);

    bool is_underlow_or_overflow(void);
#endif //AUTO_UPDATE

#if SND_MSG
    kernel_pid_t get_pid(void);
#endif //SND_MSG
private:
    float get_voltage_value(void);
    float a_factor = 1;
    float b_constant = 1;

#if AUTO_UPDATE
    bool is_under_or_overflow;
    uint16_t delta_thres = 0;
    uint16_t overflow_thres = 0;
    uint16_t underflow_thres = 0;

    uint16_t old_sensor_value = 0;

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

float linear_equation_calculate(float x_value, float a_factor,
        float b_constant);

float rational_equation_calculate(float x_value, float a_factor,
        float b_constant);

#endif //__HA_SENSOR_LINEAR_DRIVER_H_
