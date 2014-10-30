/**
 * @file ADC_device.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 21-10-2014
 * @brief This is header file for ADC device class for HA system.
 */
#ifndef __HA_ADC_DEVICE_H_
#define __HA_ADC_DEVICE_H_

#include "device_common.h"

using namespace dev_param_ns;

class adc_dev_class: private adc, private gpio {
protected:
    const uint16_t v_ref = 3300; //mV
    const uint16_t adc_value_max = 4096; //12-bits ADC

    adc_dev_class(void);

    /**
     * @brief Initialize GPIO and ADC on the given port/pin.
     *
     * @param[in] port      The port has a functional ADC.
     * @param[in] pin       The pin has a functional ADC.
     * @param[in] adc_x     Chosen ADC (ADC1, ADC2 or ADC3).
     * @param[in] channel   The specified channel on the chosen ADC.
     */
    void adc_dev_configure(port_t port, uint8_t pin, adc_t adc_x,
            uint8_t channel);

    /**
     * @brief Set up the sampling time.
     *
     * @param[in] sample_time The sampling time in cycles.
     */
    void adc_dev_sampling_time_setup(uint8_t sample_time);

    /**
     * @brief Get converted adc value.
     *
     * @return Converted ADC value.
     */
    uint16_t adc_dev_get_value(void);
private:
};

#endif //__HA_ADC_DEVICE_H_
