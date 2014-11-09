/**
 * @file sensor_event_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 7-10-2014
 * @brief This is header file for sensors making event (PIR,...) in HA system.
 */
#ifndef __HA_SENSOR_EVENT_DRIVER_H_
#define __HA_SENSOR_EVENT_DRIVER_H_

#define SND_MSG (1)

#if SND_MSG
extern "C" {
#include "msg.h"
#include "thread.h"
}
#endif //SND_MSG

#include "GPIO_device.h"

namespace sensor_event_ns {
typedef enum : uint8_t {
    low_level = 0,
    high_level = 1
} detect_level_t;

#if SND_MSG
enum
    :uint16_t {
        SEN_EVT_MSG
};
#endif //SND_MSG
}

class sensor_event_instance: private gpio_dev_class {
public:
    sensor_event_instance(void);
    ~sensor_event_instance(void);

    void device_configure(gpio_config_params_t *gpio_params);
    void set_detective_level(sensor_event_ns::detect_level_t detective_level);
    bool is_detected(void);

#if SND_MSG
    kernel_pid_t get_pid(void);
#endif //SND_MSG
private:
    uint8_t detective_level;

#if SND_MSG
    kernel_pid_t thread_pid;
#endif //SND_MSG
};

#endif //__HA_SENSOR_EVENT_DRIVER_H_
