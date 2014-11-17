/*
 * Copyright (C) 2014 Ho Chi Minh City University of Technology
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Application tests GPIO pin of MBoard 1
 *
 * @author      Nguyen Van Hien <nvhien1992@gmail.com>
 *
 * @}
 */

#include <stdio.h>

extern "C" {
#include "board.h"
#include "vtimer.h"
}

#include "MB1_System.h"

#define DELAY_TIME 100000   //100ms

typedef enum {
    port_A,
    port_B,
    port_C,
    port_D,
    port_E,
    port_F,
    port_G,
    no_port
} port_t;

uint32_t SystemCoreClock = 72000000;

static void gpio_init(GPIO_TypeDef *gpio_x, uint16_t gpio_pin, uint32_t periph_rcc);
static void test_pin_on_led(port_t port, uint8_t pin);

int main()
{
    while (1) {
        /* MBoard1 P3 start at GND-wise */
        test_pin_on_led(port_A, 7);
        test_pin_on_led(port_A, 6);
        test_pin_on_led(port_A, 5);
        test_pin_on_led(port_A, 4);
        test_pin_on_led(port_C, 11);

        /* MBoard1 P6 start at GND-wise */
        test_pin_on_led(port_B, 4);
        test_pin_on_led(port_B, 5);
        test_pin_on_led(port_B, 0);
        test_pin_on_led(port_B, 1);
        test_pin_on_led(port_C, 7);
    }
}

static void gpio_init(GPIO_TypeDef *gpio_x, uint16_t gpio_pin, uint32_t periph_rcc)
{
    RCC_APB2PeriphClockCmd(periph_rcc, ENABLE);

    GPIO_InitTypeDef gpio_init_struct;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init_struct.GPIO_Pin = gpio_pin;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpio_x, &gpio_init_struct);
}

static void test_pin_on_led(port_t port, uint8_t pin) {
    GPIO_TypeDef *gpio_x;
    uint32_t periph_rcc;
    uint16_t gpio_pin = (uint16_t)1 << pin;

    switch(port) {
    case port_A:
        gpio_x = GPIOA;
        periph_rcc = RCC_APB2Periph_GPIOA;
        break;
    case port_B:
        gpio_x = GPIOB;
        periph_rcc = RCC_APB2Periph_GPIOB;
        break;
    case port_C:
        gpio_x = GPIOC;
        periph_rcc = RCC_APB2Periph_GPIOC;
        break;
    case port_D:
        gpio_x = GPIOD;
        periph_rcc = RCC_APB2Periph_GPIOD;
        break;
    case port_E:
        gpio_x = GPIOE;
        periph_rcc = RCC_APB2Periph_GPIOE;
        break;
    case port_F:
        gpio_x = GPIOF;
        periph_rcc = RCC_APB2Periph_GPIOF;
        break;
    case port_G:
        gpio_x = GPIOG;
        periph_rcc = RCC_APB2Periph_GPIOG;
        break;
    default:
        return;
    }

    gpio_init(gpio_x, gpio_pin, periph_rcc);
    GPIO_SetBits(gpio_x, gpio_pin);
    delay_us(DELAY_TIME);
    GPIO_ResetBits(gpio_x, gpio_pin);
    delay_us(DELAY_TIME);
}
