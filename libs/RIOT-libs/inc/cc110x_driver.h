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
#ifndef __CC110X_DRIVER_H
#define __CC110X_DRIVER_H

#include "MB1_System.h"

#define GDO0__PORT	GPIOC
#define GDO0__PIN	GPIO_Pin_7
#define GDO2__PORT	GPIOC
#define GDO2__PIN	GPIO_Pin_11

#define	SPI_PORT	GPIOA
#define	NSS_PIN		GPIO_Pin_4
#define	SCLK_PIN	GPIO_Pin_5
#define MISO_PIN	GPIO_Pin_6
#define MOSI_PIN	GPIO_Pin_7

#define CC110x_MISO_LOW_RETRY        (100)        // max. retries for MISO to go low
#define CC110x_MISO_LOW_COUNT        (2700)        // loop count (timeout ~ 500 us) to wait


#endif //__CC110X_DRIVER_H

