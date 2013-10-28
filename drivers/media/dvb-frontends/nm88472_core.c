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

#include "nm88472_priv.h"


/* write single register */
int nm88472_reg(struct cxd2820r_priv *priv, u32 reg, u8 val)
{
	return 0; /* nm88472_wr_regs(priv, reg, &val, 1);*/
}

static int nm88472_init(struct dvb_frontend *fe)
{
	return 0;
}

static const struct dvb_frontend_ops nm88472_ops = {
	.delsys = { SYS_DVBT, SYS_DVBT2, SYS_DVBC_ANNEX_A },
	/* default: DVB-T/T2 */
	.info = {
		.name = "Panasonic NM88472",

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

//	.release		= nm88472_release,
	.init			= nm88472_init,
//	.sleep			= nm88472_sleep,

//	.get_tune_settings	= nm88472_get_tune_settings,
//	.i2c_gate_ctrl		= nm88472_i2c_gate_ctrl,

//	.get_frontend		= nm88472_get_frontend,

//	.get_frontend_algo	= nm88472_get_frontend_algo,
//	.search			= nm88472_search,

//	.read_status		= nm88472_read_status,
//	.read_snr		= nm88472_read_snr,
//	.read_ber		= nm88472_read_ber,
//	.read_ucblocks		= nm88472_read_ucblocks,
//	.read_signal_strength	= nm88472_read_signal_strength,
};

EXPORT_SYMBOL(nm88472_attach);

MODULE_AUTHOR("Benjamin Larsson <benjamin@southpole.se>");
MODULE_DESCRIPTION("Panasonic NM88472 demodulator driver");
MODULE_LICENSE("GPL");
