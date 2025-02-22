/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */

#ifndef _TRI_KEY_EXCEPTION_
#define _TRI_KEY_EXCEPTION_

#include <linux/version.h>

#define TRI_KEY_NOISE_TYPE       "tri_key_noise_type"
#define TRI_KEY_CALI_TYPE        "tri_key_calibration_type"

struct tri_exception_data {
	void  *private_data;
	unsigned int exception_upload_count;
};

typedef enum {
	EXCEP_DEFAULT = 0,
	EXCEP_NOISE,
	EXCEP_CAL,
	EXCEP_BUS,
	EXCEP_PROBE,
	EXCEP_RESUME,
	EXCEP_SUSPEND,
	EXCEP_IRQ,
} tri_excep_type;

int tri_key_exception_report(void *tri_exception_data, tri_excep_type excep_tpye, void *summary, unsigned int summary_size);

#endif /*_TRI_KEY_EXCEPTION_*/
