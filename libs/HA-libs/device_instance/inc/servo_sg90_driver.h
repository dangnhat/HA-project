/**
 * @file servo_sg90_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 27-10-2014
 * @brief This is header file for servo Tower Pro SG90 device instance in HA system.
 */
#ifndef __HA_SERVO_SG90_DRIVER_H_
#define __HA_SERVO_SG90_DRIVER_H_

#include "PWM_device.h"

class servo_sg90_instance: private pwm_dev_class {
public:
    servo_sg90_instance(void);

    void device_configure(pwm_config_params_t *pwm_config_params);
    void set_angle(uint8_t angle);
    void restart(void);
    void stop(void);
    uint8_t get_angle(void);
private:
    uint8_t angle;
};

#endif //__HA_SERVO_SG90_DRIVER_H_
