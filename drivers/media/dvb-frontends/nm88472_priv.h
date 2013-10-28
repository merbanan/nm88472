/*
 * Panasonic NM88472 demodulator driver
 *
 * Copyright (C) 2013 Benjamin Larsson <benjamin@southpole.se>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef NM88472_PRIV_H
#define NM88472_PRIV_H

#include <linux/dvb/version.h>
#include "dvb_frontend.h"
#include "dvb_math.h"
#include "cxd2820r.h"
#include <linux/gpio.h>

struct reg_val_mask {
	u32 reg;
	u8  val;
	u8  mask;
};

struct cxd2820r_priv {
	struct i2c_adapter *i2c;
	struct dvb_frontend fe;
	struct cxd2820r_config cfg;

	bool ber_running;

	u8 bank[3];
#define GPIO_COUNT 3
	u8 gpio[GPIO_COUNT];
#ifdef CONFIG_GPIOLIB
	struct gpio_chip gpio_chip;
#endif

	fe_delivery_system_t delivery_system;
	bool last_tune_failed; /* for switch between T and T2 tune */
};

#endif /* NM88472_PRIV_H */