/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */

#ifndef _TRIKEY_HEALTHONFO_
#define _TRIKEY_HEALTHONFO_

#define KEVENT_LOG_TAG              "psw_bsp_tp"
#define KEVENT_EVENT_ID             "tp_fatal_report"
#define HISTORY_MAX                 8

typedef enum {
	HEALTH_INTERF = 1,
	HEALTH_IRQ_DATA,
	HEALTH_KEY_UPLOAD_STATE,
	HEALTH_STATE_COUNT,
} healthinfo_type;

struct dhall_data_xyz_m {
	short hall_x;
	short hall_y;
	short hall_z;
	int hall_v;
	u8 st;
};

struct history_data{
	int interf_type;
	short		state;
	short		dhall_data0;
	short		dhall_data1;
	s64 msecs64;
	s64 secs64;
	struct dhall_data_xyz_m hall_value;
	u64 his_data_curr_count;
	short key_upload_state;
};

struct monitor_data {
	void  *chip_data; /*debug info data*/
	bool health_monitor_support;
	bool threeaxis_hall_support;

	/*state count*/
	u64 up_state_count;
	u64 mid_state_count;
	u64 down_state_count;
	short last_state;
	/*noise*/
	u64 interf_1_count;
	u64 interf_2_count;
	u64 interf_3_count;
	u64 interf_4_count;
	u64 interf_5_count;
	u64 interf_6_count;
	u64 interf_7_count;
	u64 interf_8_count;
	/*history_data*/
	struct history_data his_data[HISTORY_MAX];
	unsigned int his_data_index;
	u64 his_data_count;
	/*suspend*/
	u64 pm_resume_count;
	u64 pm_suspend_count;
	u64 irq_need_dev_resume_all_count;
};

int tri_healthinfo_report(void *data, healthinfo_type type,
			void *value);

int tri_healthinfo_read(struct seq_file *s, void *data);

int tri_healthinfo_clear(void *data);

int tri_healthinfo_init(struct device *dev, void *data);

#endif /* _TRIKEY_HEALTHONFO_ */
