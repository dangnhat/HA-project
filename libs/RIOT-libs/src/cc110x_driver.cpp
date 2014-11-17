/*
 * Copyright (C) 2014 Ho Chi Minh City University of Technology
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     mboard1
 * @{
 *
 * @file
 * @brief       Low level diver for cc110x.
 *
 * @author      Hien Van Nguyen <nvhien1992@gmail.com>
 */
#include <stdio.h>
#include <stddef.h>

extern "C" {
#include "cpu.h"
#include "sched.h"
#include "vtimer.h"

/* drivers  */
#include "cc110x_ng.h"
}

#include "cc110x_driver.h"

/* Global vars changed if MB1 system has been changed */
gpio *gdo0 = &MB1_EXTI7;
gpio *gdo2 = &MB1_EXTI11;
SPI *spi_x = &MB1_SPI1;
ISRMgr *isr_mgr = &MB1_ISRs;

ISRMgr_ns::ISR_t gdo0_isr_type = ISRMgr_ns::ISRMgr_EXTI7;
ISRMgr_ns::ISR_t gdo2_isr_type = ISRMgr_ns::ISRMgr_EXTI11;

#define CC110x_GET_GDO0()           (gdo0->gpio_read())
#define CC110x_GET_GDO2()           (gdo2->gpio_read())
#define CC110x_GET_GDO1_MISO()      (spi_x->misc_MISO_read())

#define SPI_SELECT()        (spi_x->SM_device_select(SPI_ns::cc1101_1))
#define SPI_UNSELECT()      (spi_x->SM_device_deselect(SPI_ns::cc1101_1))

static void gdo_init_interrupt(void);
static void nss_init(void);
static void gdo0_irq(void *arg);
static void gdo2_irq(void *arg);

int cc110x_get_gdo0(void)
{
    return CC110x_GET_GDO0();
}

int cc110x_get_gdo1(void)
{
    return CC110x_GET_GDO1_MISO();
}

int cc110x_get_gdo2(void)
{
    return CC110x_GET_GDO2();
}

static void nss_init(void)
{
    spi_x->SM_numOfSSLines_set(1); //There is 1 chip select line on SPI1.

    SPI_ns::SM_GPIOParams_s params_struct;

    params_struct.GPIO_port = SPI_PORT;
    params_struct.GPIO_pin = NSS_PIN;
    params_struct.GPIO_clk = RCC_APB2Periph_GPIOA;
    params_struct.ssLine = 0;
    spi_x->SM_GPIO_set(&params_struct);

    spi_x->SM_deviceToDecoder_set(SPI_ns::cc1101_1, 0);
    spi_x->SM_deviceToDecoder_set(SPI_ns::allFree, 1);
}

void cc110x_spi_init(void)
{
    nss_init();

    SPI_ns::SPI_params_t params_struct;

    params_struct.mode = SPI_Mode_Master;
    params_struct.direction = SPI_Direction_2Lines_FullDuplex;
    params_struct.CPOL = SPI_CPOL_Low;
    params_struct.CPHA = SPI_CPHA_1Edge;
    params_struct.dataSize = SPI_DataSize_8b;
    params_struct.firstBit = SPI_FirstBit_MSB;
    params_struct.baudRatePrescaler = SPI_BaudRatePrescaler_16;
    params_struct.crcPoly = 7;
    params_struct.nss = SPI_NSS_Soft;

    spi_x->init(&params_struct);
    spi_x->SM_device_attach(SPI_ns::cc1101_1);
}

uint8_t cc110x_txrx(uint8_t value)
{
    return spi_x->M2F_sendAndGet_blocking(SPI_ns::cc1101_1, value);
}

void cc110x_spi_cs(void)
{
    SPI_SELECT();
}

void cc110x_spi_unselect(void)
{
    SPI_UNSELECT();
}

void cc110x_spi_select(void)
{
    volatile int retry_count = 0;
    volatile int abort_count;

    cc110x_spi_cs();    //CS to low
    while (CC110x_GET_GDO1_MISO() != 0) {
        cc110x_spi_cs();
        abort_count++;
        if (abort_count > CC110x_MISO_LOW_COUNT) {
            retry_count++;
            if (retry_count > CC110x_MISO_LOW_RETRY) {
                puts("[CC110x SPI] fatal error\n");
                return;
            }
            cc110x_spi_unselect();  // CS to high
        }
    } //end while
}

void cc110x_before_send(void)
{
    //Disable GDO2 interrupt before sending packet
    cc110x_gdo2_disable();
}

void cc110x_after_send(void)
{
    //Enable GDO2 interrupt after sending packet
    cc110x_gdo2_enable();
}

void cc110x_gdo0_enable(void)
{
    gdo0->exti_line_enable();
}

void cc110x_gdo2_enable(void)
{
    gdo2->exti_line_enable();
}

void cc110x_gdo0_disable(void)
{
    gdo0->exti_line_disable();
}

void cc110x_gdo2_disable(void)
{
    gdo2->exti_line_disable();
}

static void gdo0_irq(void *arg)
{
    cc110x_gdo0_irq();
}

static void gdo2_irq(void *arg)
{
    cc110x_gdo2_irq();
}

static void gdo_init_interrupt(void)
{
    gpio_ns::gpio_params_t params_struct;

    /* Initialize interrupt GDO0 */
    params_struct.port = GDO0_PORT_SRC;
    params_struct.pin = GDO0_PIN_SRC;
    params_struct.gpio_speed = gpio_ns::speed_10MHz;
    params_struct.mode = gpio_ns::in_floating;
    gdo0->gpio_init(&params_struct);
    gdo0->exti_init(gpio_ns::rising_edge);

    /* Initialize interrupt GDO2 */
    params_struct.port = GDO2_PORT_SRC;
    params_struct.pin = GDO2_PIN_SRC;
    gdo2->gpio_init(&params_struct);
    gdo2->exti_init(gpio_ns::falling_edge);

    isr_mgr->subISR_EXTI_assign(gdo0_isr_type, gdo0_irq, NULL);
    isr_mgr->subISR_EXTI_assign(gdo2_isr_type, gdo2_irq, NULL);
}

void cc110x_init_interrupts(void)
{
    gdo_init_interrupt();
}

