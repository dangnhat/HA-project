/**
 * @file ledw.cpp
 * @author  Do Mai Anh Tu <aberrant.accolades@gmail.com>
 * @version 1.0
 * @date 4/12/2013
 * @copyright
 * @brief
 * Implement some source functions of:
 * LED one color
 * Dimming led
 * Led with large power
 */

/***********************************************************************/

#include "stm32f10x.h"
#include "ledw.h"

/*Global Variables*/
TIM_OCInitTypeDef TIM_OCInitStruct;
uint16_t TimerPeriod = 0, ChannelPulse = 0;
static uint16_t DutyCycle = 0;

/**
  * @attention Change values of parameters that needs to be replace by another values here
  */

/*LED parameters*/
GPIO_TypeDef* LEDW_PORT = GPIOB;
const uint16_t LEDW_PIN = GPIO_Pin_9;
const uint32_t LEDW_CLK = RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO;
GPIOMode_TypeDef LEDW_MODE = GPIO_Mode_AF_PP;
GPIOSpeed_TypeDef LEDW_SPEED = GPIO_Speed_50MHz;

/*TIM parameters*/
TIM_TypeDef* TIM_TYPE = TIM4;
const uint32_t TIM_CLK = RCC_APB1Periph_TIM4;
void (*TIM_RCC_set) (uint32_t, FunctionalState) = (TIM_TYPE == TIM2 || TIM_TYPE == TIM3 || TIM_TYPE == TIM4) ?  RCC_APB1PeriphClockCmd : RCC_APB2PeriphClockCmd;

/*NVIC parameters*/
const uint32_t PriorityGroup = NVIC_PriorityGroup_3;
const uint8_t IQRChannel = TIM4_IRQn;
const uint8_t PreemptionPriorityValue = 0;
const uint8_t SubPriorityValue = 1;

/**
  * @brief Configure GPIO, CLOCK for LED
  * @param None
  * @return None
  * @attention None
  */
void LEDW_c::LEDW_Configuration(){
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(LEDW_CLK,ENABLE);

    GPIO_InitStruct.GPIO_Pin = LEDW_PIN;
    GPIO_InitStruct.GPIO_Mode = LEDW_MODE;
    GPIO_InitStruct.GPIO_Speed = LEDW_SPEED;
    GPIO_Init(LEDW_PORT, &GPIO_InitStruct);

}



/**
  * @brief Configure clock of TIM that will be use
  * @param None
  * @return None
  * @attention None
  */
void LEDW_c::TIM_Configuration(){
    TIM_RCC_set(TIM_CLK, ENABLE);
}

/**
  * @brief Configure Nested Vector Interrupt Table
  * @param None
  * @return None
  * @attention None
  */
void LEDW_c::NVIC_Configuration(void) {
    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_PriorityGroupConfig(PriorityGroup);

    NVIC_InitStruct.NVIC_IRQChannel = IQRChannel;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = PreemptionPriorityValue;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = SubPriorityValue;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief Initial TIM for PWM purpose
  * @param None
  * @return None
  * @attention If Channel of TIM that be used not fit, remember to reconfigure
  */
void LEDW_c::TIM_Init(){
    TIM_Configuration();

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    DutyCycle = 0;
    TimerPeriod = (SystemCoreClock / 50000) - 1;
    ChannelPulse = (uint16_t) (((uint32_t) DutyCycle * (TimerPeriod - 1))/ 100);

    TIM_TimeBaseStruct.TIM_Prescaler = 0;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseStruct.TIM_Period = TimerPeriod;
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM_TYPE, &TIM_TimeBaseStruct);


    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = ChannelPulse;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC4Init(TIM_TYPE, &TIM_OCInitStruct);

    TIM_Cmd(TIM_TYPE, ENABLE);

    TIM_CtrlPWMOutputs(TIM_TYPE, ENABLE);
}

/**
  * @brief Enable one high power single led
  * @param None
  * @return None
  * @attention This function have to be called only one time and before other functions
  */
void LEDW_c::Start() {
    LEDW_Configuration();
    NVIC_Configuration();
    TIM_Init();
}

/**
  * @brief Disable one high power single led
  * @param None
  * @return None
  * @attention None
  */
void LEDW_c::Shutdown() {
    RCC_APB2PeriphClockCmd(LEDW_CLK,DISABLE);
    TIM_RCC_set(TIM_CLK, DISABLE);
    TIM_Cmd(TIM_TYPE, DISABLE);
    TIM_CtrlPWMOutputs(TIM_TYPE, DISABLE);
}

/**
  * @brief Increase duty cycle
  * @param None
  * @return None
  * @attention If Channel of TIM that be used not fit, remember to reconfigure
  * @attention This function usually goes with a button
  */
void LEDW_c::Increase_Duty(void) {
    if (DutyCycle == 100 ) DutyCycle = 0;
    else DutyCycle += 10;
    ChannelPulse = (uint16_t) (((uint32_t) (DutyCycle) * (TimerPeriod - 1))/ 100);
    TIM_OCInitStruct.TIM_Pulse = ChannelPulse;
    TIM_OC4Init(TIM_TYPE, &TIM_OCInitStruct);
}


