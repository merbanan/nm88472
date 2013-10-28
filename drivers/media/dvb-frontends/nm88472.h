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

#ifndef NM88472_H
#define NM88472_H

#include <linux/kconfig.h>
#include <linux/dvb/frontend.h>


struct nm88472_config {
	/* Demodulator I2C address.
	 * Default: none, must set
	 * Values: 0x38  (0x1c) - DVB-T2
     *         0x30  (0x18) - DVB-T
     *         0x34  (0x1a) - DVB-C
	 */
	u8 i2c_address;

	/* TS output mode.
	 * Default: none, must set.
	 * Values:
	 */
	u8 ts_mode;
};

#if IS_ENABLED(CONFIG_DVB_NM88472)
extern struct dvb_frontend *nm88472_attach(
	const struct nm88472_config *config,
	struct i2c_adapter *i2c,
	int *gpio_chip_base
);
#else
static inline struct dvb_frontend *nm88472_attach(
	const struct nm88472_config *config,
	struct i2c_adapter *i2c,
	int *gpio_chip_base
)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}

#endif

#endif /* NM88472_H */