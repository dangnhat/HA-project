/**
 * @file dimmer_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for dimmer device instance for HA system.
 */
#ifndef __HA_DIMMER_DRIVER_H_
#define __HA_DIMMER_DRIVER_H_

/* 0 if want to get value manually */
#define AUTO_UPDATE (1)
#if AUTO_UPDATE

/* 1 if want to send msg to its thread */
#define SND_MSG (1)
#if SND_MSG
extern "C" {
#include "msg.h"
#include "thread.h"
}
#endif //SND_MSG

#endif //AUTO_UPDATE

#include "ADC_device.h"

namespace dimmer_ns {
#if SND_MSG
enum
    : uint16_t {
        DIMMER_MSG
};
#endif
}

class dimmer_instance: private adc_dev_class {
public:
    dimmer_instance(void);
    ~dimmer_instance(void);

    void device_configure(adc_config_params_t *adc_config_params);
    uint8_t get_percent(void);
#if AUTO_UPDATE
    /**
     *
     */
    uint8_t dimmer_processing(void);

    /**
     *
     */
    bool is_over_delta_thres(void);
#endif

#if SND_MSG
    kernel_pid_t get_pid(void);
    bool is_first_send = true;
#endif
private:
    adc_config_params_t adc_params;
#if AUTO_UPDATE
    uint8_t new_value_1 = 0;
    uint8_t new_value_2 = 0;
    uint8_t new_value_3 = 0;
    uint8_t old_value;
    bool is_over_delta;

    void assign_dimmer(void);
    void remove_dimmer(void);
#endif

#if SND_MSG
    kernel_pid_t thread_pid;
#endif
};

#if AUTO_UPDATE
void dimmer_callback_timer_isr(void);
#endif

#endif //__HA_DIMMER_DRIVER_H_
