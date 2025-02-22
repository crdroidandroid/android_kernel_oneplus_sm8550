/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */

#ifndef _TRIKEY_COMMON_API_
#define _TRIKEY_COMMON_API_

/*******Part0:LOG TAG Declear************************/
extern unsigned int tri_key_debug;

enum debug_level {
	LEVEL_BASIC,
	LEVEL_DEBUG,
};

#define TRI_KEY_DEVICE "oplus,hall_tri_state_key"
#define TRI_KEY_TAG                  "[tri_state_key] "

#define TRI_KEY_ERR(fmt, args...)\
	pr_err(TRI_KEY_TAG" %s : "fmt, __func__, ##args)
#define TRI_KEY_LOG(fmt, args...)\
	pr_err(TRI_KEY_TAG" %s : "fmt, __func__, ##args)
#define TRI_KEY_DEBUG(fmt, args...)\
	do {\
		if (tri_key_debug == LEVEL_DEBUG)\
			pr_info(TRI_KEY_TAG " %s: " fmt, __func__, ##args);\
	} while (0)

#endif /* _TRIKEY_COMMON_API_ */
