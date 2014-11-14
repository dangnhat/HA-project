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
 * @file        cc110x_reconfig.h
 * @brief       Header file: Re-configure ouput power (PATABLE) and frequency carrier for MBoard1.
 *
 * @author      Hien Van Nguyen <nvhien1992@gmail.com>
 */
#ifndef __CC110X_RECONFIG_H
#define __CC110X_RECONFIG_H

/**
 * @brief Set up output power manually
 */
void cc110x_set_up_patable(uint8_t PA_value);

/**
 * @brief Set up frequency carrier manually
 */
void cc110x_set_up_freq_carrier(uint8_t freq2, uint8_t freq1, uint8_t freq0);

/**
 * @brief Re-configure Configuration register for CC110x at 390MHz
 */
void cc110x_reconfig(void);

/**
 * @brief Re-configure Configuration register for CC110x at 390MHz (only affects
 *  on FREQ2, FREQ1, FREQ0) and PA at 0 dBm.
 */
void cc110x_reconfig390(void);

#endif /* __CC110X_RECONFIG_H */
/** @} */
