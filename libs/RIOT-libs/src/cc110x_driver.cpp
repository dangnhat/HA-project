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

#define CC110x_GET_GDO0()			(GPIO_ReadInputDataBit(GDO0__PORT, GDO0__PIN))
#define CC110x_GET_GDO2()			(GPIO_ReadInputDataBit(GDO2__PORT, GDO2__PIN))
#define CC110x_GET_GDO1_MISO()		(MB1_SPI1.misc_MISO_read())

#define SPI_SELECT()        (MB1_SPI1.SM_device_select(SPI_ns::cc1101_1))
#define SPI_UNSELECT()      (MB1_SPI1.SM_device_deselect(SPI_ns::cc1101_1))

static void gdo_init_interrupt(void);
static void nss_init(void);

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
    MB1_SPI1.SM_numOfSSLines_set(1); //There is 1 chip select line on SPI1.

    SPI_ns::SM_GPIOParams_s params_struct;

    params_struct.GPIO_port = SPI_PORT;
    params_struct.GPIO_pin = NSS_PIN;
    params_struct.GPIO_clk = RCC_APB2Periph_GPIOA;
    params_struct.ssLine = 0;
    MB1_SPI1.SM_GPIO_set(&params_struct);

    MB1_SPI1.SM_deviceToDecoder_set(SPI_ns::cc1101_1, 0);
    MB1_SPI1.SM_deviceToDecoder_set(SPI_ns::allFree, 1);
}

void cc110x_spi_init(void)
{
//    printf("spi init\n");
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

    MB1_SPI1.init(&params_struct);
    MB1_SPI1.SM_device_attach(SPI_ns::cc1101_1);
}

uint8_t cc110x_txrx(uint8_t value)
{
    return MB1_SPI1.M2F_sendAndGet_blocking(SPI_ns::cc1101_1, value);
}

void cc110x_spi_cs(void)
{
    SPI_SELECT();
    printf("nss cs: %d\n", GPIO_ReadInputDataBit(SPI_PORT, NSS_PIN));
}

void cc110x_spi_unselect(void)
{
    SPI_UNSELECT();
    printf("nss uncs: %d\n", GPIO_ReadInputDataBit(SPI_PORT, NSS_PIN));
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
    MB1_EXTI7.exti_line_enable();
}

void cc110x_gdo2_enable(void)
{
    MB1_EXTI11.exti_line_enable();
}

void cc110x_gdo0_disable(void)
{
    MB1_EXTI7.exti_line_disable();
}

void cc110x_gdo2_disable(void)
{
    MB1_EXTI11.exti_line_disable();
}

static void gdo_init_interrupt(void)
{
    exti_ns::exti_params_t params_struct;

    /* Initialize interrupt GDO0 */
    params_struct.port = exti_ns::port_C;
    params_struct.mode = exti_ns::floating;
    params_struct.trigger = exti_ns::rising_edge;
    MB1_EXTI7.exti_init(&params_struct);

    /* Initialize interrupt GDO2 */
    params_struct.trigger = exti_ns::falling_edge;
    MB1_EXTI11.exti_init(&params_struct);

    MB1_ISRs.subISR_assign(ISRMgr_ns::ISRMgr_EXTI7, &cc110x_gdo0_irq);
    MB1_ISRs.subISR_assign(ISRMgr_ns::ISRMgr_EXTI11,  &cc110x_gdo2_irq);
}

void cc110x_init_interrupts(void)
{
    gdo_init_interrupt();
    printf("init interrupt\n");
}

