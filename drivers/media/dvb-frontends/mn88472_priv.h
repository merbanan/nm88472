/*
 * Panasonic MN88472 DVB-T/T2/C demodulator driver
 *
 * Copyright (C) 2013 Antti Palosaari <crope@iki.fi>
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
 */

#ifndef MN88472_PRIV_H
#define MN88472_PRIV_H

#include "dvb_frontend.h"
#include "dvb_math.h"
#include "mn88472.h"
#include <linux/firmware.h>

#define MN88472_FIRMWARE "dvb-demod-mn88472-02.fw"

struct mn88472_priv {
	struct i2c_adapter *i2c;
	const struct mn88472_config *cfg;
	struct dvb_frontend fe;
	fe_delivery_system_t delivery_system;
	bool warm; /* FW running */
};

/* mn88472_core.c */
int mn88472_wregs(struct mn88472_priv *s, u16 reg, const u8 *val, int len);
int mn88472_rregs(struct mn88472_priv *s, u16 reg, u8 *val, int len);
int mn88472_rreg(struct mn88472_priv *s, u16 reg, u8 *val);
int mn88472_wreg(struct mn88472_priv *s, u16 reg, u8 val);

/* mn88472_c.c */
int mn88472_read_status_c(struct dvb_frontend *fe, fe_status_t *status);
int mn88472_set_frontend_c(struct dvb_frontend *fe);

/* mn88472_t.c */
int mn88472_set_frontend_t(struct dvb_frontend *fe);
int mn88472_read_status_t(struct dvb_frontend *fe, fe_status_t *status);

#endif /* MN88472_PRIV_H */
