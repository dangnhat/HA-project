/**
 * @file rgb.h
 * @author  Do Mai Anh Tu <aberrant.accolades@gmail.com>
 * @version 1.0
 * @date 15/10/2013
 * @copyright
 * @brief
 * This includes a class that involves some prototype functions of LED RGB for STM32.
 */

/***********************************************************************/

#ifndef RGB_H_INCLUDED
#define RGB_H_INCLUDED

#include "stm32f10x.h"

class RGB_c {
private :
    void RGB_Configuration(void);
    void TIM_Configuration(void);
    void RGB_8bit_Init(void);
public:
    void Start();
    void Shutdown();
    void RGB_Init(uint32_t CounterClock, uint32_t TimerClock);
    void SetColor(uint8_t red, uint8_t green, uint8_t blue);
};
#endif // RGB_H_INCLUDED
