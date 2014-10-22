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

#define GDO0_PORT_SRC   (gpio_ns::port_C)
#define GDO2_PORT_SRC   (gpio_ns::port_C)
#define GDO0_PIN_SRC    (7)
#define GDO2_PIN_SRC    (11)

#define	SPI_PORT        (GPIOA)
#define	NSS_PIN         (GPIO_Pin_4)

#define CC110x_MISO_LOW_RETRY        (100)        // max. retries for MISO to go low
#define CC110x_MISO_LOW_COUNT        (2700)        // loop count (timeout ~ 500 us) to wait

#endif //__CC110X_DRIVER_H

