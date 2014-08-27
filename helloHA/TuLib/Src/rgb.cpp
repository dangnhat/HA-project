/**
 * @file rgb.cpp
 * @author  Do Mai Anh Tu <aberrant.accolades@gmail.com>
 * @version 1.0
 * @date 15/10/2013
 * @copyright
 * @brief Implement some source functions of LED RGB for STM32
 */

/***********************************************************************/

#include "rgb.h"

#define PIN_SIZE 3

/*Global Variables*/
TIM_OCInitTypeDef  RGB_TIM_OCInitStruct;
uint16_t PeriodValue = 0;

/**
  * @attention Change values of parameters that needs to be replace by another values here
  */

/*LED RGB parameters*/
GPIO_TypeDef* RGB_PORT[PIN_SIZE] = {GPIOB, GPIOB, GPIOB};
const uint16_t RGB_Pin[PIN_SIZE] = {GPIO_Pin_9, GPIO_Pin_8, GPIO_Pin_7};
const uint32_t RGB_CLK[PIN_SIZE] = {RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO};
GPIOMode_TypeDef RGB_MODE[PIN_SIZE] = {GPIO_Mode_AF_PP, GPIO_Mode_AF_PP, GPIO_Mode_AF_PP};
GPIOSpeed_TypeDef RGB_SPEED[PIN_SIZE] = {GPIO_Speed_10MHz, GPIO_Speed_10MHz, GPIO_Speed_10MHz};

/* TIM parameters*/
TIM_TypeDef* RGB_TIM[3] = {TIM4, TIM4, TIM4};
const uint32_t RGB_TIM_CLK[3] = {RCC_APB1Periph_TIM4, RCC_APB1Periph_TIM4, RCC_APB1Periph_TIM4};
void (*RGB_TIM_RCC_Set[PIN_SIZE]) (uint32_t, FunctionalState) = {RCC_APB1PeriphClockCmd, RCC_APB1PeriphClockCmd, RCC_APB2PeriphClockCmd};

/*PWM parameters*/
void (*RGB_TIM_OC_Init[PIN_SIZE]) (TIM_TypeDef*, TIM_OCInitTypeDef*) = {TIM_OC2Init, TIM_OC3Init, TIM_OC4Init};
void (*RGB_TIM_OC_PreloadConfig[PIN_SIZE]) (TIM_TypeDef*, uint16_t) = {TIM_OC2PreloadConfig, TIM_OC3PreloadConfig, TIM_OC4PreloadConfig};


/**
  * @brief Configure GPIO, CLOCK for each pin of RGB LED
  * @param None
  * @return None
  * @attention This function have to be called only one time and before other functions
  */
void RGB_c::RGB_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    uint8_t i;
    for (i = 0; i < PIN_SIZE ; i++) {
        GPIO_InitStruct.GPIO_Pin = RGB_Pin[i];
        GPIO_InitStruct.GPIO_Mode = RGB_MODE[i];
        GPIO_InitStruct.GPIO_Speed = RGB_SPEED[i];
        GPIO_Init(RGB_PORT[i], &GPIO_InitStruct);
        RCC_APB2PeriphClockCmd(RGB_CLK[i] , ENABLE);
    }
}

/**
  * @brief Configure appropriate clock for each TIM that set for each pin of RGB led
  * @param None
  * @return None
  * @attention This function have to be called only one time and before other functions
  */
void RGB_c::TIM_Configuration() {
    uint8_t i;
    for (i = 0; i < PIN_SIZE; i++)
        RGB_TIM_RCC_Set[i](RGB_TIM_CLK[i],ENABLE);
}

/**
  * @brief Initial one RGB LED
  * @param CounterClock number of clock to calculate prescaler value
  * @param TimerClock number of clock to calculate period value
  * @return None
  * @attention If
  */
void RGB_c::RGB_Init(uint32_t CounterClock, uint32_t TimerClock) {

    RGB_Configuration();
    TIM_Configuration();

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    uint16_t PrescalerValue = 0;
    uint8_t i;
    PrescalerValue = (uint16_t)((SystemCoreClock / CounterClock) - 1);
    PeriodValue = (uint16_t)((CounterClock / TimerClock) - 1);

    TIM_TimeBaseStruct.TIM_Prescaler = PrescalerValue;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseStruct.TIM_Period = PeriodValue;
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;

    RGB_TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    RGB_TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    RGB_TIM_OCInitStruct.TIM_Pulse = 0;
    RGB_TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

    for (i = 0; i < PIN_SIZE; i++) {
        TIM_TimeBaseInit(RGB_TIM[i], &TIM_TimeBaseStruct);

        RGB_TIM_OC_Init[i](RGB_TIM[i], &RGB_TIM_OCInitStruct);
        RGB_TIM_OC_PreloadConfig[i](RGB_TIM[i], TIM_OCPreload_Enable);

        TIM_ARRPreloadConfig(RGB_TIM[i], ENABLE);
        TIM_Cmd(RGB_TIM[i], ENABLE);
        TIM_CtrlPWMOutputs(RGB_TIM[i], ENABLE);
    }
}

/**
  * @brief Initial one RGB LED that uses 8 bits to calculate the duty cycle of each color
  * @param None
  * @return None
  * @attention None
  */
void RGB_c::RGB_8bit_Init() {
    RGB_Init(8000000,31250);
}

/**
  * @brief Enable one led RGB
  * @param None
  * @return None
  * @attention This function have to be called only one time and before other functions
  */
void RGB_c::Start() {
    RGB_8bit_Init();
}

void RGB_c::Shutdown() {
    uint8_t i;

    for (i = 0; i < PIN_SIZE; i++) {
        RCC_APB2PeriphClockCmd(RGB_CLK[i] , DISABLE);
        RGB_TIM_RCC_Set[i](RGB_TIM_CLK[i],DISABLE);
        TIM_ARRPreloadConfig(RGB_TIM[i], DISABLE);
        TIM_Cmd(RGB_TIM[i], DISABLE);
        TIM_CtrlPWMOutputs(RGB_TIM[i], DISABLE);
    }
}

/**
  * @brief Set one color for RGB LED
  * @param 8 bit value represents RGB value of desired color
  * @return None
  * @attention This calculation of pulse just applies for led RGB with common anode, if otherwise change 255 - x (red,green,blue) = x respectively
  */
void RGB_c::SetColor(uint8_t red, uint8_t green, uint8_t blue) {
    uint8_t i;
    for (i = 0; i < PIN_SIZE; i++) {
        RGB_TIM_OCInitStruct.TIM_Pulse = ( i == 0 ) ? (uint16_t)(255 - red) : ((i == 1) ? (uint16_t)(255 - green) : (uint16_t)(255 - blue));
        RGB_TIM_OC_Init[i](RGB_TIM[i], &RGB_TIM_OCInitStruct);
        RGB_TIM_OC_PreloadConfig[i](RGB_TIM[i], TIM_OCPreload_Enable);
    }
}
