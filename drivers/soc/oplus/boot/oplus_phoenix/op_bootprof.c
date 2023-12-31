// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */

/*=============================================================================

                            INCLUDE FILES FOR MODULE

=============================================================================*/
#include "op_bootprof.h"
#include "oplus_phoenix.h"
#include <asm/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/utsname.h>

#define BOOT_STR_SIZE 256
#define BOOT_LOG_NUM 192
//#define TRACK_TASK_COMM
#define BOOT_FROM_SIZE 16

extern unsigned long long notrace sched_clock(void);

struct boot_log_struct {
	/* task cmdline for first 16 bytes
* and boot event for the rest
* if TRACK_TASK_COMM is on */
	char *comm_event;
#ifdef TRACK_TASK_COMM
	pid_t pid;
#endif
	u64 timestamp;
} op_bootprof[BOOT_LOG_NUM];

static int boot_log_count;
static DEFINE_MUTEX(op_bootprof_lock);
static bool op_bootprof_enabled;
static bool show_timestamp = false;
static int boot_finish = 0;
char boot_from[BOOT_FROM_SIZE] = { '\0' };

#define MSG_SIZE 128

/*
 * Ease the printing of nsec fields:
*/

static long long nsec_high(unsigned long long nsec)
{
	if ((long long)nsec < 0) {
		nsec = -nsec;
		do_div(nsec, 1000000);
		return -nsec;
	}
	do_div(nsec, 1000000);

	return nsec;
}

static unsigned long nsec_low(unsigned long long nsec)
{
	if ((long long)nsec < 0) {
		nsec = -nsec;
	}

	return do_div(nsec, 1000000);
}

void op_log_boot(const char *str)
{
	unsigned long long ts;
	struct boot_log_struct *p = &op_bootprof[boot_log_count];
	size_t n = strlen(str) + 1;

	if (!op_bootprof_enabled) {
		return;
	}
	ts = sched_clock();
	if (boot_log_count >= BOOT_LOG_NUM) {
		pr_err("[BOOTPROF] not enuough bootprof buffer\n");
		return;
	}
	mutex_lock(&op_bootprof_lock);
	p->timestamp = ts;
#ifdef TRACK_TASK_COMM
	p->pid = current->pid;
	n += TASK_COMM_LEN;
#endif
	p->comm_event = kzalloc(n, GFP_ATOMIC | __GFP_NORETRY | __GFP_NOWARN);
	if (!p->comm_event) {
		op_bootprof_enabled = false;
		goto out;
	}
#ifdef TRACK_TASK_COMM
	memcpy(p->comm_event, current->comm, TASK_COMM_LEN);
	memcpy(p->comm_event + TASK_COMM_LEN, str, n - TASK_COMM_LEN);
#else
	memcpy(p->comm_event, str, n);
#endif
	boot_log_count++;
out:
	mutex_unlock(&op_bootprof_lock);
}

static void op_bootprof_switch(int on)
{
	mutex_lock(&op_bootprof_lock);
	if (op_bootprof_enabled ^ on) {
		unsigned long long ts = sched_clock();

		pr_info("BOOTPROF:%10ld.%06ld: %s\n", nsec_high(ts),
			nsec_low(ts), on ? "ON" : "OFF");

		if (on) {
			op_bootprof_enabled = 1;
		} else {
			/* boot up complete */
			boot_finish = 1;
		}
	}
	mutex_unlock(&op_bootprof_lock);
}

static ssize_t op_bootprof_write(struct file *filp, const char *ubuf,
				 size_t cnt, loff_t *data)
{
	char buf[BOOT_STR_SIZE];
	size_t copy_size = cnt;

	if (cnt >= sizeof(buf)) {
		copy_size = BOOT_STR_SIZE - 1;
	}

	if (copy_from_user(&buf, ubuf, copy_size)) {
		return -EFAULT;
	}

	if (cnt == 2 && buf[0] == '1') {
		op_bootprof_switch(1);
		return cnt;
	} else if (cnt == 2 && buf[0] == '0') {
		op_bootprof_switch(0);
		return cnt;
	} else if (cnt == 2 && buf[0] == 'T') {
		show_timestamp = !show_timestamp;
		return cnt;
	}

	buf[copy_size] = 0;
	phx_monit(buf);

	return cnt;
}

static ssize_t op_bootfrom_write(struct file *filp, const char *ubuf,
				 size_t cnt, loff_t *data)
{
	size_t copy_size = cnt;

	if (cnt >= sizeof(boot_from)) {
		copy_size = BOOT_FROM_SIZE - 1;
	}

	if (copy_from_user(&boot_from, ubuf, copy_size)) {
		return -EFAULT;
	}

	boot_from[copy_size] = 0;

	return cnt;
}

static int op_bootprof_show(struct seq_file *m, void *v)
{
	int i;
	struct boot_log_struct *p;

	for (i = 0; i < boot_log_count; i++) {
		p = &op_bootprof[i];
		if (!p->comm_event) {
			continue;
		}

#define FMT "%s\n"
#define FMT_TIME "%6ld.%06ld : %s\n"
#define FMT_FULL "%10Ld.%06ld :%5d-%-16s: %s\n"

#ifdef TRACK_TASK_COMM
		SEQ_printf(m, FMT_FULL,
			nsec_high(p->timestamp), nsec_low(p->timestamp),
			p->pid, p->comm_event, p->comm_event + TASK_COMM_LEN);
#else
		if (show_timestamp) {
			SEQ_printf(m, FMT_TIME,
				nsec_high(p->timestamp),
				nsec_low(p->timestamp),
				p->comm_event);
		} else {
			SEQ_printf(m, FMT, p->comm_event);
		}
#endif /* TRACK_TASK_COMM */
	}
	return 0;
}

static int op_bootfrom_show(struct seq_file *m, void *v)
{
	SEQ_printf(m, "%s\n", boot_from);
	return 0;
}

static int op_bootprof_open(struct inode *inode, struct file *file)
{
	return single_open(file, op_bootprof_show, inode->i_private);
}

static int op_bootfrom_open(struct inode *inode, struct file *file)
{
	return single_open(file, op_bootfrom_show, inode->i_private);
}

static const struct proc_ops op_bootprof_fops = {
	.proc_open = op_bootprof_open,
	.proc_write = op_bootprof_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static const struct proc_ops op_bootfrom_fops = {
	.proc_open = op_bootfrom_open,
	.proc_write = op_bootfrom_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

/* device_initcall */
void __init init_boot_prof(void)
{
	proc_create("phoenix", 0666, NULL, &op_bootprof_fops);
	proc_create("opbootfrom", 0666, NULL, &op_bootfrom_fops);
}

/* early_initcall */
void __init init_bootprof_buf(void)
{
	op_bootprof_switch(1);
}
