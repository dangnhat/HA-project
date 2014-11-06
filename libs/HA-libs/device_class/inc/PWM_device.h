/**
 * @file PWM_device.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 22-10-2014
 * @brief This is header file for PWM device class for HA system.
 */
#ifndef __HA_PWM_DEVICE_H_
#define __HA_PWM_DEVICE_H_

#include "device_common.h"

using namespace dev_param_ns;

class pwm_dev_class: private pwm_hw, private gpio {
protected:
    pwm_dev_class(void);

    /**
     * @brief Initialize GPIO and TIM to be able to use PWM function.
     *
     * @param[in] port          The port has a functional ADC.
     * @param[in] pin           The pin has a functional ADC.
     * @param[in] timer         The TIM chosen to generate PWM.
     * @param[in] pwm_channel   The timer channel on the chosen TIM.
     */
    void pwm_dev_configure(port_t port, uint8_t pin, pwm_timer_t timer,
            uint8_t pwm_channel);

    /**
     * @brief Start or stop device by controlling TIM_OCx.
     *
     * @param[in] is_started Started if true, stopped otherwise.
     */
    void pwm_dev_start_stop(bool is_started);

    /**
     * @brief Set the period for PWM.
     *
     * @param[in] period 0->65535.
     */
    void pwm_dev_period_setup(uint16_t period);

    /**
     * @brief Set duty_cycle pulse time in range 0->period.
     *
     * @param[in] level The timer_pulses.
     */
    void pwm_dev_level_setup(uint16_t level);

    /**
     * @brief Set duty_cycle in percent value (0->100%).
     *
     * param[in] duty_cycle
     */
    void pwm_dev_duty_cycle_setup(uint8_t duty_cycle);

    /**
     * @brief Set input frequency for timer generating PWM. (TIM clock counter)
     *
     * @param[in] freq_in_hz The frequency of timer in Hz.
     */
    void pwm_dev_timer_frequency_setup(uint32_t freq_in_hz);

    /**
     * @brief Set output frequency on pwm channel based on period.
     *
     * @param[in] freq_in_hz The given output frequency in Hz.
     */
    void pwm_dev_output_frequency_setup(uint32_t freq_in_hz);

    /**
     * @brief Convert the given duty_cycle to timer pulses.
     *
     * @param[in] duty_cycle Duty cycle value in range (0-100%).
     *
     * @return timer_pulses.
     */
    uint16_t timer_pulse_convert(uint8_t duty_cycle);

    /**
     * @brief Convert the given timer pulses to duty_cycle.
     *
     * @param[in] timer_pulses The high level time of pwm in range (0->period).
     *
     * @return duty_cycle.
     */
    uint8_t duty_cycle_convert(uint16_t timer_pulses);
private:
};

#endif //__HA_PWM_DEVICE_H_
