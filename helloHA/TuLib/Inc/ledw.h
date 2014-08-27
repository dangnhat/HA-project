/**
 * @file ledw.h
 * @author  Do Mai Anh Tu <aberrant.accolades@gmail.com>
 * @version 1.0
 * @date 4/12/2013
 * @copyright
 * @brief
 * Implement some prototype functions of:
 * LED one color
 * Dimming led
 * Led with large power
 */

/***********************************************************************/

#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

class LEDW_c {
private:
    void LEDW_Configuration();
    void TIM_Configuration();
    void NVIC_Configuration();
    void TIM_Init();
public:
    void Start();
    void Shutdown();
    void Increase_Duty();
};

#endif // TIMER_H_INCLUDED
