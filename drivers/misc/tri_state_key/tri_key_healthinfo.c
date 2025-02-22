// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */

#include <linux/err.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/ktime.h>
#include <asm/current.h>
#include <linux/version.h>
#include <linux/seq_file.h>

#include "tri_key_healthinfo.h"
#include "tri_key_common_api.h"

static void tri_state_count_monitor_handle(void *data, void *value)
{
	struct monitor_data *p_monitor_data = (struct monitor_data *)data;
	short *state = (short *)value;

	if (p_monitor_data->last_state != *state) {
		if (1 == *state) {
			p_monitor_data->up_state_count++;
		}
		if (2 == *state) {
			p_monitor_data->mid_state_count++;
		}
		if (3 == *state) {
			p_monitor_data->down_state_count++;
		}
	}
	p_monitor_data->last_state = *state;
	return;
}

static void tri_irq_data_monitor_handle(void *data, void *value)
{
	struct monitor_data *p_monitor_data = (struct monitor_data *)data;
	struct history_data *history = (struct history_data *)value;


	if (p_monitor_data->his_data_index < HISTORY_MAX) {
		memcpy(&p_monitor_data->his_data[p_monitor_data->his_data_index],
			history, sizeof(struct history_data));
		/*count history irq data*/
		p_monitor_data->his_data_count++;
		p_monitor_data->his_data[p_monitor_data->his_data_index].his_data_curr_count = p_monitor_data->his_data_count;
	}
	/*record current index*/
	p_monitor_data->his_data_index++;
	if (p_monitor_data->his_data_index >= HISTORY_MAX) {
		p_monitor_data->his_data_index = 0;
	}

	return;
}

static void tri_key_upload_state_monitor_handle(void *data, void *value)
{
	struct monitor_data *p_monitor_data = (struct monitor_data *)data;
	short *key_upload_state = (short *)value;

	if (p_monitor_data->his_data_index < HISTORY_MAX) {
		p_monitor_data->his_data[p_monitor_data->his_data_index].key_upload_state = *key_upload_state;
	}
	return;
}

int tri_healthinfo_report(void *data, healthinfo_type type,
			 void *value)
{
	struct monitor_data *p_monitor_data = (struct monitor_data *)data;

	if (!p_monitor_data || !p_monitor_data->health_monitor_support) {
		TRI_KEY_ERR("data is null!\n");
		return -EINVAL;
	}

	switch (type) {
	case HEALTH_STATE_COUNT:
		tri_state_count_monitor_handle(data, value);
		break;
	case HEALTH_IRQ_DATA:
		tri_irq_data_monitor_handle(data, value);
		break;
	case HEALTH_KEY_UPLOAD_STATE:
		tri_key_upload_state_monitor_handle(data, value);
		break;
	default:
		break;
	}

	return 0;
}
EXPORT_SYMBOL(tri_healthinfo_report);

int tri_healthinfo_read(struct seq_file *s, void *data)
{
	struct monitor_data *p_monitor_data = (struct monitor_data *)data;
	int index = 0;
	u64 curr_count = 0;

	if (!p_monitor_data || !p_monitor_data->health_monitor_support) {
		TRI_KEY_ERR("data is null!\n");
		return -EINVAL;
	}

	if (p_monitor_data->up_state_count) {
		seq_printf(s, "tri_up_state_count:%llu\n", p_monitor_data->up_state_count);
	}

	if (p_monitor_data->mid_state_count) {
		seq_printf(s, "tri_mid_state_count:%llu\n", p_monitor_data->mid_state_count);
	}

	if (p_monitor_data->down_state_count) {
		seq_printf(s, "tri_down_state_count:%llu\n", p_monitor_data->down_state_count);
	}

	if (p_monitor_data->pm_resume_count) {
		seq_printf(s, "tri_pm_resume_count:%llu\n", p_monitor_data->pm_resume_count);
	}

	if (p_monitor_data->pm_suspend_count) {
		seq_printf(s, "tri_pm_suspend_count:%llu\n", p_monitor_data->pm_suspend_count);
	}

	if (p_monitor_data->irq_need_dev_resume_all_count) {
		seq_printf(s, "tri_irq_need_dev_resume_all_count:%llu\n", p_monitor_data->irq_need_dev_resume_all_count);
	}

	for (index = 0; index < HISTORY_MAX && p_monitor_data->his_data_count > index;
		index++) {
		curr_count = p_monitor_data->his_data[index].his_data_curr_count;
		seq_printf(s, "tri_interf_type_%llu:%d\n", curr_count, p_monitor_data->his_data[index].interf_type);
		seq_printf(s, "tri_state_%llu:%hu\n", curr_count, p_monitor_data->his_data[index].state);
		seq_printf(s, "tri_times_%llu:%llu\n", curr_count, p_monitor_data->his_data[index].msecs64);
		seq_printf(s, "tri_key_upload_state_%llu:%hu\n", curr_count, p_monitor_data->his_data[index].key_upload_state);
		if (!p_monitor_data->threeaxis_hall_support) {
			seq_printf(s, "tri_dhall_data0_%llu:%hu\n", curr_count, p_monitor_data->his_data[index].dhall_data0);
			seq_printf(s, "tri_dhall_data1_%llu:%hu\n", curr_count, p_monitor_data->his_data[index].dhall_data1);
		} else {
			seq_printf(s, "tri_hall_x_%llu:%hu\n", curr_count, p_monitor_data->his_data[index].hall_value.hall_x);
			seq_printf(s, "tri_hall_y_%llu:%hu\n", curr_count, p_monitor_data->his_data[index].hall_value.hall_y);
			seq_printf(s, "tri_hall_z_%llu:%hu\n", curr_count, p_monitor_data->his_data[index].hall_value.hall_z);
			seq_printf(s, "tri_hall_v_%llu:%d\n", curr_count, p_monitor_data->his_data[index].hall_value.hall_v);
			seq_printf(s, "tri_st_%llu:%hu\n", curr_count, p_monitor_data->his_data[index].hall_value.st);
		}
	}
	return 0;
}

int tri_healthinfo_clear(void *data)
{
	struct monitor_data *p_monitor_data = (struct monitor_data *)data;

	if (!p_monitor_data || !p_monitor_data->health_monitor_support) {
		TRI_KEY_ERR("data is null!\n");
		return -EINVAL;
	}

	TRI_KEY_LOG("Clear health info Now!\n");
	p_monitor_data->up_state_count = 0;
	p_monitor_data->mid_state_count = 0;
	p_monitor_data->down_state_count = 0;

	p_monitor_data->interf_1_count = 0;
	p_monitor_data->interf_2_count = 0;
	p_monitor_data->interf_3_count = 0;
	p_monitor_data->interf_4_count = 0;
	p_monitor_data->interf_5_count = 0;
	p_monitor_data->interf_6_count = 0;
	p_monitor_data->interf_7_count = 0;
	p_monitor_data->interf_8_count = 0;

	memset(p_monitor_data->his_data, 0, sizeof(struct history_data) * HISTORY_MAX);
	p_monitor_data->his_data_count = 0;
	p_monitor_data->his_data_index = 0;

	TRI_KEY_LOG("Clear health info Finish!\n");

	return 0;
}

int tri_healthinfo_init(struct device *dev, void *data)
{
	struct monitor_data *p_monitor_data = (struct monitor_data *)data;

	if (!p_monitor_data || !p_monitor_data->health_monitor_support) {
		TRI_KEY_ERR("data is null!\n");
		return -EINVAL;
	}
	return 0;
}
