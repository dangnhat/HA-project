/**
 * @file sensor_event_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 7-10-2014
 * @brief This is source file for sensors making event (PIR,...) in HA system.
 */
#include "sensor_event_driver.h"

using namespace sensor_event_ns;

static void sensor_callback(void *arg);

sensor_event_instance::sensor_event_instance(void) :
        gpio_dev_class(true)
{
    /* activate in high level, default */
    this->detective_level = 1;
#if SND_MSG
    this->thread_pid = thread_getpid();
#endif //SND_MSG
}

sensor_event_instance::~sensor_event_instance(void)
{
    gpio_dev_remove_callback();
}

void sensor_event_instance::device_configure(gpio_config_params_t *gpio_params)
{
    gpio_dev_configure(gpio_params->device_port, gpio_params->device_pin,
            gpio_params->mode);
    gpio_dev_int_both_edge();
    gpio_dev_assign_callback(&sensor_callback, this);
}

void sensor_event_instance::set_detective_level(detect_level_t detective_level)
{
    this->detective_level = (uint8_t) detective_level;
}

bool sensor_event_instance::is_detected(void)
{
    uint8_t status = gpio_dev_read();
    if ((status * detective_level > 0) || status == detective_level) {
        return true;
    }

    return false;
}

#if SND_MSG
kernel_pid_t sensor_event_instance::get_pid(void)
{
    return this->thread_pid;
}
#endif //SND_MSG

static void sensor_callback(void *arg)
{
    sensor_event_instance * sensor = (sensor_event_instance*) arg;

#if SND_MSG
    msg_t msg;
    msg.type = SEN_EVT_MSG;
    if (sensor->is_detected()) {
        msg.content.value = sensor_event_ns::high_level;
    } else {
        msg.content.value = sensor_event_ns::low_level;
    }

    /* check valid pid and send msg */
    kernel_pid_t pid = sensor->get_pid();
    if (pid == KERNEL_PID_UNDEF) {
        return;
    }
    msg_send(&msg, pid, false);
#endif //SND_MSG
}
