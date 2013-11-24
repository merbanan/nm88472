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

/* Very much based on cxd2820r demod code Copyright (C) 2010 Antti Palosaari <crope@iki.fi> */

#define DEBUG
#include "nm88472_priv.h"
#define dev_dbg dev_err


static const struct dvb_frontend_ops nm88472_ops;

/* write single register */
int nm88472_wr_reg(struct nm88472_priv *priv, u8 bank_adr, u8 reg, u8 val)
{
	int ret;
	u8 buf[2];
	struct i2c_msg msg[1] = {
		{
			.addr = bank_adr,
			.flags = 0,
			.len = 2,
			.buf = buf,
		}
	};

	buf[0] = reg;
	buf[1] = val;

	ret = i2c_transfer(priv->i2c, msg, 1);
	if (ret == 1) {
		ret = 0;
	} else {
		dev_warn(&priv->i2c->dev, "%s: i2c wr failed=%d i2c_adr=%02x " \
				"reg=%02x\n", KBUILD_MODNAME, ret, bank_adr, reg);
		ret = -EREMOTEIO;
	}
	return ret;
	
}

int nm88472_rd_reg(struct nm88472_priv *priv, u8 bank_adr, u8 reg, u8 *val)
{
	int ret;
	u8 buf;
	struct i2c_msg msg[2] = {
		{
			.addr = bank_adr,
			.flags = 0,
			.len = 1,
			.buf = &reg,
		}, {
			.addr = bank_adr,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = &buf,
		}
	};

	ret = i2c_transfer(priv->i2c, msg, 2);
	if (ret == 2) {
		*val = buf;
		ret = 0;
	} else {
		dev_warn(&priv->i2c->dev, "%s: i2c rd failed=%d reg=%02x", \
				KBUILD_MODNAME, ret, reg);
		ret = -EREMOTEIO;
	}

	return ret;}

int nm88472_wr_table(struct nm88472_priv *priv, const struct nm88472_i2c_reg_byte* table, int tab_length)
{
	int i, ret = 0;
	for (i=0 ; i<tab_length ; i++) {
		ret |= nm88472_wr_reg(priv, table[i].i2c_bank, table[i].addr, table[i].val);
//		msleep(20);
	}
	return ret;
}

int nm88472_set_frontend_c(struct dvb_frontend *fe)
{
	struct nm88472_priv *priv = fe->demodulator_priv;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	int ret = 0;

	dev_info(&priv->i2c->dev, "%s: %s", KBUILD_MODNAME,  __func__);	


	dev_dbg(&priv->i2c->dev, "%s: frequency=%d symbol_rate=%d\n", __func__,
			c->frequency, c->symbol_rate);

	/* program tuner */
	if (fe->ops.tuner_ops.set_params)
		fe->ops.tuner_ops.set_params(fe);

	priv->delivery_system = SYS_DVBC_ANNEX_A;

	/* demod register settings */
	ret |= nm88472_wr_table(priv, demod_pre_conf, ARRAY_SIZE(demod_pre_conf));
	ret |= nm88472_wr_reg(priv, T2, 0x03, SYS_DVB_C);
	ret |= nm88472_wr_reg(priv, T2, 0x04, SYS_BW_8MHZ);
	ret |= nm88472_wr_table(priv, dvbc_8MHz_tab, ARRAY_SIZE(dvbc_8MHz_tab));
	if (ret)
		goto error;

	return ret;

error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}


int nm88472_set_frontend_t(struct dvb_frontend *fe)
{
	struct nm88472_priv *priv = fe->demodulator_priv;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	int ret = 0;

	dev_info(&priv->i2c->dev, "%s: %s", KBUILD_MODNAME,  __func__);	

	dev_dbg(&priv->i2c->dev, "%s: frequency=%d symbol_rate=%d\n", __func__,
			c->frequency, c->symbol_rate);

	/* program tuner */
	if (fe->ops.tuner_ops.set_params)
		fe->ops.tuner_ops.set_params(fe);

	priv->delivery_system = SYS_DVBT;
	
	/* demod register settings */
	ret |= nm88472_wr_table(priv, demod_pre_conf, ARRAY_SIZE(demod_pre_conf));
	ret |= nm88472_wr_reg(priv, T2, 0x03, SYS_DVB_T);

	switch (c->bandwidth_hz) {
	case 6000000:
		ret |= nm88472_wr_reg(priv, T2, 0x04, SYS_BW_6MHZ);
		ret |= nm88472_wr_table(priv, dvbt_6MHz_tab, ARRAY_SIZE(dvbt_6MHz_tab));
		break;
	case 7000000:
		ret |= nm88472_wr_reg(priv, T2, 0x04, SYS_BW_7MHZ);
		ret |= nm88472_wr_table(priv, dvbt_7MHz_tab, ARRAY_SIZE(dvbt_7MHz_tab));
		break;
	case 8000000:
		ret |= nm88472_wr_reg(priv, T2, 0x04, SYS_BW_8MHZ);
		ret |= nm88472_wr_table(priv, dvbt_8MHz_tab, ARRAY_SIZE(dvbt_8MHz_tab));
		break;
	default:
		return -EINVAL;
	}
	ret |= nm88472_wr_table(priv, dvbt_tab_post, ARRAY_SIZE(dvbt_tab_post));
	
	if (ret)
		goto error;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

int nm88472_set_frontend_t2(struct dvb_frontend *fe)
{
	struct nm88472_priv *priv = fe->demodulator_priv;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	int ret = 0;
	
	dev_info(&priv->i2c->dev, "%s: %s", KBUILD_MODNAME,  __func__);	

	dev_dbg(&priv->i2c->dev, "%s: frequency=%d symbol_rate=%d\n", __func__,
			c->frequency, c->symbol_rate);

	/* program tuner */
	if (fe->ops.tuner_ops.set_params)
		fe->ops.tuner_ops.set_params(fe);

	priv->delivery_system = SYS_DVBT2;

	/* demod register settings */
	ret |= nm88472_wr_table(priv, demod_pre_conf, ARRAY_SIZE(demod_pre_conf));
	ret |= nm88472_wr_reg(priv, T2, 0x03, SYS_DVB_T2);

	switch (c->bandwidth_hz) {
	case 5000000:
		ret |= nm88472_wr_reg(priv, T2, 0x04, SYS_BW_5MHZ);
		ret |= nm88472_wr_table(priv, dvbt2_5MHz_tab, ARRAY_SIZE(dvbt2_5MHz_tab));
		break;
	case 6000000:
		ret |= nm88472_wr_reg(priv, T2, 0x04, SYS_BW_6MHZ);
		ret |= nm88472_wr_table(priv, dvbt2_6MHz_tab, ARRAY_SIZE(dvbt2_6MHz_tab));
		break;
	case 7000000:
		ret |= nm88472_wr_reg(priv, T2, 0x04, SYS_BW_7MHZ);
		ret |= nm88472_wr_table(priv, dvbt2_7MHz_tab, ARRAY_SIZE(dvbt2_7MHz_tab));
		break;
	case 8000000:
		ret |= nm88472_wr_reg(priv, T2, 0x04, SYS_BW_8MHZ);
		ret |= nm88472_wr_table(priv, dvbt2_8MHz_tab, ARRAY_SIZE(dvbt2_8MHz_tab));
		break;
	default:
		return -EINVAL;
	}
	ret |= nm88472_wr_table(priv, dvbt2_tab_post, ARRAY_SIZE(dvbt2_tab_post));
	
	if (ret)
		goto error;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

/*
static int nm88472_read_ucblocks(struct dvb_frontend *fe, u32 *ucblocks)
{
	struct nm88472_priv *priv = fe->demodulator_priv;
	int ret;

	dev_dbg(&priv->i2c->dev, "%s: delsys=%d\n", __func__,
			fe->dtv_property_cache.delivery_system);

	switch (fe->dtv_property_cache.delivery_system) {
	case SYS_DVBT:
		ret = nm88472_read_ucblocks_t(fe, ucblocks);
		break;
	case SYS_DVBT2:
		ret = nm88472_read_ucblocks_t2(fe, ucblocks);
		break;
	case SYS_DVBC_ANNEX_A:
		ret = nm88472_read_ucblocks_c(fe, ucblocks);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}
*/


static int nm88472_initialize_demod(struct nm88472_priv *priv)
{
	int i, ret = 0;
	const struct firmware *fw;
	u8 val;
	u8 *fw_file = NM88472_FIRMWARE;

	dev_info(&priv->i2c->dev, "%s: %s", KBUILD_MODNAME,  __func__);	

	dev_info(&priv->i2c->dev, "%s: found a '%s', will try " \
			"to load a firmware\n",
			KBUILD_MODNAME, nm88472_ops.info.name);

	/* request the firmware, this will block and timeout */
	ret = request_firmware(&fw, fw_file, priv->i2c->dev.parent);
	if (ret) {
		dev_info(&priv->i2c->dev, "%s: did not find the firmware " \
			"file. (%s) Please see linux/Documentation/dvb/ for " \
			"more details on firmware-problems. (%d)\n",
			KBUILD_MODNAME, fw_file, ret);
		goto err;
	}

	dev_info(&priv->i2c->dev, "%s: downloading firmware from file '%s'\n",
			KBUILD_MODNAME, fw_file);

	/* Initialize all bank registers */
	ret = nm88472_wr_table(priv, demod_bank_init, ARRAY_SIZE(demod_bank_init));
	if (ret)
		goto err_release;

	/* Prepare DVB-T bank for firmware upload */
	ret |= nm88472_wr_reg(priv, T1, 0xf0, 0x20);
	/* Reset mcu */
	ret |= nm88472_wr_reg(priv, T1, 0xf5, 0x03);

	if (ret)
		goto err_release;

	for (i = 0; i < fw->size; i++) {
		ret |= nm88472_wr_reg(priv, T1, 0xf6, fw->data[i]);
//		msleep(20);
	}
	if (ret)
		goto err_release;

	/* Check firmware parity */
	ret |= nm88472_rd_reg(priv, T1, 0xf8, &val);
	if (ret)
		goto err_release;

	if (val & 0x10) {
		dev_info(&priv->i2c->dev, "%s: firmware '%s' parity check failed\n",
				KBUILD_MODNAME, fw_file);
		goto err_release;
	}

	/* Start mcu with loaded firmware */
	ret |= nm88472_wr_reg(priv, T1, 0xf5, 0x00);
	if (ret)
		goto err_release;

	priv->demod_init = 1;
err_release:
	release_firmware(fw);
err:
	if (ret)
		dev_info(&priv->i2c->dev, "%s: '%s' demod init failed\n",
				KBUILD_MODNAME, nm88472_ops.info.name);
	if (!ret)
		dev_info(&priv->i2c->dev, "%s: '%s' demod initialized\n",
				KBUILD_MODNAME, nm88472_ops.info.name);
	return ret;
}


static int nm88472_set_serial_ts_mode(struct nm88472_priv *priv)
{
	int ret;
	
	dev_info(&priv->i2c->dev, "%s: %s", KBUILD_MODNAME,  __func__);	

	switch (priv->cfg.ts_mode) {
	case PARALLEL_FIXED_CLOCK:
		ret  = nm88472_wr_reg(priv, T2, 0x08, 0x00);
		ret |= nm88472_wr_reg(priv, T1, 0xd9, 0xe1);
		break;
	case PARALLEL_VARIABLE_CLOCK:
		ret  = nm88472_wr_reg(priv, T2, 0x08, 0x00);
		ret |= nm88472_wr_reg(priv, T1, 0xd9, 0xe3);
		break;
	case SERIAL_VARIABLE_CLOCK:
		ret  = nm88472_wr_reg(priv, T2, 0x08, 0x1d);
		ret |= nm88472_wr_reg(priv, T1, 0xd9, 0xe3);
		break;
	default:
		return -EINVAL;
	}

	return ret;
}

static int nm88472_init(struct dvb_frontend *fe)
{
	int ret;
	struct nm88472_priv *priv;
	priv = fe->demodulator_priv;
//	dev_info(&priv->i2c->dev, "%s: Init", KBUILD_MODNAME);	
	if (priv->demod_init)
		return 0;

	ret  = nm88472_initialize_demod(priv);
	ret |= nm88472_set_serial_ts_mode(priv);
	return ret;
}

static void nm88472_release(struct dvb_frontend *fe)
{
	struct nm88472_priv *priv = fe->demodulator_priv;

	kfree(priv);
	return;
}

static int nm88472_set_frontend(struct dvb_frontend *fe)
{
	struct nm88472_priv *priv = fe->demodulator_priv;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	int ret;

	dev_dbg(&priv->i2c->dev, "%s: delsys=%d\n", __func__,
	fe->dtv_property_cache.delivery_system);

	switch (c->delivery_system) {
	case SYS_DVBT:
		ret = nm88472_set_frontend_t(fe);
		if (ret < 0)
			goto err;
		break;
	case SYS_DVBT2:
		ret = nm88472_set_frontend_t2(fe);
		if (ret < 0)
			goto err;
		break;
	case SYS_DVBC_ANNEX_A:
		ret = nm88472_set_frontend_c(fe);
		if (ret < 0)
			goto err;
		break;
	default:
		dev_dbg(&priv->i2c->dev, "%s: error state=%d\n", __func__,
			fe->dtv_property_cache.delivery_system);
		ret = -EINVAL;
		break;
	}
	err:
	return ret;
}

static enum dvbfe_search nm88472_search(struct dvb_frontend *fe)
{
	struct nm88472_priv *priv = fe->demodulator_priv;
//	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	int ret;
//	fe_status_t status = 0;

	dev_dbg(&priv->i2c->dev, "%s: delsys=%d\n", __func__,
		fe->dtv_property_cache.delivery_system);
#if 0
	/* switch between DVB-T and DVB-T2 when tune fails */
	if (priv->last_tune_failed) {
		if (priv->delivery_system == SYS_DVBT) {
			ret = cxd2820r_sleep_t(fe);
		if (ret)
			goto error;

		c->delivery_system = SYS_DVBT2;
		} else if (priv->delivery_system == SYS_DVBT2) {
			ret = cxd2820r_sleep_t2(fe);
			if (ret)
				goto error;

			c->delivery_system = SYS_DVBT;
		}
	}
#endif
	/* set frontend */
	ret = nm88472_set_frontend(fe);
	if (ret)
	  goto error;

	/* hack */
	return DVBFE_ALGO_SEARCH_SUCCESS;

#if 0
	/* frontend lock wait loop count */
	switch (priv->delivery_system) {
	case SYS_DVBT:
	case SYS_DVBC_ANNEX_A:
	  i = 20;
	  break;
	case SYS_DVBT2:
	  i = 40;
	  break;
	case SYS_UNDEFINED:
	default:
	  i = 0;
	  break;
	}

	/* wait frontend lock */
	for (; i > 0; i--) {
	  dev_dbg(&priv->i2c->dev, "%s: loop=%d\n", __func__, i);
	  msleep(50);
	  ret = cxd2820r_read_status(fe, &status);
	  if (ret)
	    goto error;

	  if (status & FE_HAS_LOCK)
	    break;
	}

	/* check if we have a valid signal */
	if (status & FE_HAS_LOCK) {
	  priv->last_tune_failed = 0;
	  return DVBFE_ALGO_SEARCH_SUCCESS;
	} else {
	  priv->last_tune_failed = 1;
	  return DVBFE_ALGO_SEARCH_AGAIN;
	}
#endif
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return DVBFE_ALGO_SEARCH_ERROR;
}

static int nm88472_get_frontend_algo(struct dvb_frontend *fe)
{
	return DVBFE_ALGO_CUSTOM;
}

static const struct dvb_frontend_ops nm88472_ops = {
	.delsys = { SYS_DVBT, SYS_DVBT2, SYS_DVBC_ANNEX_A },
	/* default: DVB-T/T2 */
	.info = {
		.name = "Panasonic NM88472",
		.frequency_min = 47000000,
		.frequency_max = 865000000,
		/* For DVB-C */
		.symbol_rate_min = 870000,
		.symbol_rate_max = 11700000,
		/* For DVB-T */
		.frequency_stepsize = 166667,
		.caps =	FE_CAN_FEC_1_2			|
			FE_CAN_FEC_2_3			|
			FE_CAN_FEC_3_4			|
			FE_CAN_FEC_5_6			|
			FE_CAN_FEC_7_8			|
			FE_CAN_FEC_AUTO			|
			FE_CAN_QPSK			|
			FE_CAN_QAM_16			|
			FE_CAN_QAM_32			|
			FE_CAN_QAM_64			|
			FE_CAN_QAM_128			|
			FE_CAN_QAM_256			|
			FE_CAN_QAM_AUTO			|
			FE_CAN_TRANSMISSION_MODE_AUTO	|
			FE_CAN_GUARD_INTERVAL_AUTO	|
			FE_CAN_HIERARCHY_AUTO		|
			FE_CAN_MUTE_TS			|
			FE_CAN_2G_MODULATION		|
			FE_CAN_MULTISTREAM
		},

 	.release		= nm88472_release,
	.init			= nm88472_init,
//	.sleep			= nm88472_sleep,

//	.get_tune_settings	= nm88472_get_tune_settings,
//	.get_frontend		= nm88472_get_frontend,
	.get_frontend_algo	= nm88472_get_frontend_algo,
	.search				= nm88472_search,

//	.read_status		= nm88472_read_status,
//	.read_snr		= nm88472_read_snr,
//	.read_ber		= nm88472_read_ber,
//	.read_ucblocks		= nm88472_read_ucblocks,
//	.read_signal_strength	= nm88472_read_signal_strength,
};


struct dvb_frontend *nm88472_attach(const struct nm88472_config *cfg,
		struct i2c_adapter *i2c )
{
	struct nm88472_priv *priv;
	int ret;

	priv = kzalloc(sizeof(struct nm88472_priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		dev_err(&i2c->dev, "%s: kzalloc() failed\n",
				KBUILD_MODNAME);
		goto error;
	}

	priv->i2c = i2c;
	memcpy(&priv->cfg, cfg, sizeof(struct nm88472_config));
	memcpy(&priv->fe.ops, &nm88472_ops, sizeof(struct dvb_frontend_ops));
	priv->fe.demodulator_priv = priv;

	dev_info(&priv->i2c->dev, "%s: Attaching nm88472", KBUILD_MODNAME);	

	return &priv->fe;
error:
	dev_dbg(&i2c->dev, "%s: failed=%d\n", __func__, ret);
	kfree(priv);
	return NULL;
}


EXPORT_SYMBOL(nm88472_attach);

MODULE_AUTHOR("Benjamin Larsson <benjamin@southpole.se>");
MODULE_DESCRIPTION("Panasonic NM88472 demodulator driver");
MODULE_LICENSE("GPL");
MODULE_FIRMWARE(NM88472_FIRMWARE);
