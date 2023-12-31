#ifndef _OPTIGA_TYPE_H_
#define _OPTIGA_TYPE_H_
#include <linux/ctype.h>
//#include <stdio.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/time.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/version.h>
#include "oplus_optiga_batt_def.h"

//typedef signed char BYTE;
//typedef unsigned char UBYTE;
//typedef unsigned char uint8_t;

//typedef short WORD;
//typedef unsigned short UWORD;
//typedef unsigned short uint16_t;

//typedef long LONG;
//typedef unsigned long ULONG;
//typedef unsigned long uint32_t;

typedef unsigned char BOOL;

struct optiga_test_result {
	int test_count_total;
	int test_count_now;
	int test_fail_count;
	int real_test_count_now;
	int real_test_fail_count;
};

struct optiga_hmac_status {
	int authenticate_result;
	int fail_count;
	int total_count;
	int real_fail_count;
	int real_total_count;
};

struct optiga_batt_vendor_info {
	int vendor_index; /* 0:first vendor, 1:second vendor */
	int module_vendor; /* battery module vendor */
	int core_vendor; /* battery core vendor */
	int core_limited_vol;
};

#define BARCODE_SUBSTR_SIZE 13
#define MAX_BARCODE_NUM 6

struct optiga_batt_info {
	struct optiga_batt_vendor_info vendor_data;
	char barcode_str[BARCODE_SUBSTR_SIZE];
};

struct oplus_optiga_chip {
	struct device *dev;
	struct pinctrl *pinctrl;
	struct pinctrl_state *optiga_active;
	int cpu_id;
	int key_id;
	int data_gpio;
	bool support_optiga_in_lk;
	bool barcode_read_support;
	int barcode_num;
	/* at most 6 barcodes.
	 * every barcodes according to a
	 * (vendor_index module_vendor core_vendor core_limited_vol)
	 */
	struct optiga_batt_info batt_data[MAX_BARCODE_NUM];
	int right_barcode_index;
	int try_count;
	spinlock_t slock;
	struct optiga_test_result test_result;
	struct optiga_hmac_status hmac_status;
	struct completion	is_complete;
	struct delayed_work auth_work;
	struct delayed_work test_work;
};

int get_optiga_pin(void);
void set_optiga_pin_dir(uint8_t dir);
void set_optiga_pin(uint8_t level);
bool oplus_optiga_get_init_done(void);
int oplus_get_batt_id_use_barcode(void);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
int oplus_optiga_driver_init(void);
void oplus_optiga_driver_exit(void);
#endif

struct oplus_optiga_chip * oplus_get_optiga_info (void);

#define MAX_DEGREE (163)
#define ARRAY_LEN(A) (((A)+31)/32)

typedef uint32_t dwordvec_t[ARRAY_LEN(MAX_DEGREE)];

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif
