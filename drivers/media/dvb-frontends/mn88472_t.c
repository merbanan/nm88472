/*
 * Panasonic MN88472 DVB-T/T2/C demodulator driver
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
 */

#include "mn88472_priv.h"

int mn88472_set_frontend_t(struct dvb_frontend *fe)
{
	struct mn88472_priv *s = fe->demodulator_priv;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	int ret;
	u32 if_frequency = 0;
	dev_dbg(&s->i2c->dev,
			"%s: delivery_system=%d modulation=%d frequency=%d symbol_rate=%d inversion=%d\n",
			__func__, c->delivery_system, c->modulation,
			c->frequency, c->symbol_rate, c->inversion);

	if (!s->warm) {
		ret = -EAGAIN;
		goto err;
	}

	/* program tuner */
	if (fe->ops.tuner_ops.set_params) {
		ret = fe->ops.tuner_ops.set_params(fe);
		if (ret)
			goto err;
	}

	if (fe->ops.tuner_ops.get_if_frequency) {
		ret = fe->ops.tuner_ops.get_if_frequency(fe, &if_frequency);
		if (ret)
			goto err;

		dev_dbg(&s->i2c->dev, "%s: get_if_frequency=%d\n",
				__func__, if_frequency);
	}

	if (if_frequency != 5070000) {
		dev_err(&s->i2c->dev, "%s: IF frequency %d not supported\n",
				KBUILD_MODNAME, if_frequency);
		ret = -EINVAL;
		goto err;
	}

	
	/* Set ts mode to serial */
	ret = mn88472_wregs(s, 0x1c08, "\x1d", 1);
	if (ret)
		goto err;

	ret = mn88472_wregs(s, 0x18d9, "\xe3", 1);
	if (ret)
		goto err;


	/* Generic init for DVB-T*/
	ret = mn88472_wregs(s, 0x1c83, "\x01", 1);
	if (ret)
		goto err;

	ret = mn88472_wregs(s, 0x1c00, "\x66\x00\x01\x02", 4);
	if (ret)
		goto err;

	switch (c->bandwidth_hz) {
	case 6000000:
		ret = mn88472_wregs(s, 0x1c04, "\x02", 1);
		if (ret)
			goto err;
		
		ret = mn88472_wregs(s, 0x1c10, "\x2c\x94\xdb\xbf\x55\x55\x15\x6b\x15\x6b", 10);
		if (ret)
			goto err;

		ret = mn88472_wregs(s, 0x1846, "\x00", 1);
		if (ret)
			goto err;
		break;
	case 7000000:
		ret = mn88472_wregs(s, 0x1c04, "\x01", 1);
		if (ret)
			goto err;
		
		ret = mn88472_wregs(s, 0x1c10, "\x39\x11\xbc\xa4\x00\x00\x0f\x2c\x0f\x2c", 10);
		if (ret)
			goto err;

		ret = mn88472_wregs(s, 0x1846, "\x10", 1);
		if (ret)
			goto err;
	case 8000000:
		ret = mn88472_wregs(s, 0x1c04, "\x00", 1);
		if (ret)
			goto err;

		ret = mn88472_wregs(s, 0x1c10, "\x39\x11\xbc\x8f\x80\x00\x08\xee\x08\xee", 10);
		if (ret)
			goto err;

		ret = mn88472_wregs(s, 0x1846, "\x00", 1);
		if (ret)
			goto err;
	default:
		return -EINVAL;
	}

	ret = mn88472_wregs(s, 0x18ae, "\x00", 1);
	if (ret)
		goto err;
	ret = mn88472_wregs(s, 0x18b0, "\x0a", 1);
	if (ret)
		goto err;
	ret = mn88472_wregs(s, 0x18b4, "\x00", 1);
	if (ret)
		goto err;
	ret = mn88472_wregs(s, 0x18cd, "\x1f", 1);
	if (ret)
		goto err;
	ret = mn88472_wregs(s, 0x18d4, "\x0a", 1);
	if (ret)
		goto err;
	ret = mn88472_wregs(s, 0x18d6, "\x48", 1);
	if (ret)
		goto err;

	s->delivery_system = c->delivery_system;

	return 0;
err:
	dev_dbg(&s->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

int mn88472_read_status_t(struct dvb_frontend *fe, fe_status_t *status)
{
	struct mn88472_priv *s = fe->demodulator_priv;
	int ret;
	u8 u8tmp;

	*status = 0;

	if (!s->warm) {
		ret = -EAGAIN;
		goto err;
	}

	ret = mn88472_rreg(s, 0x1874, &u8tmp);
	if (ret)
		goto err;

	if ((u8tmp&0x0f) > 8)
		*status = FE_HAS_SIGNAL | FE_HAS_CARRIER | FE_HAS_VITERBI |
				FE_HAS_SYNC | FE_HAS_LOCK;

	return 0;
err:
	dev_dbg(&s->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}
