/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_INPUT_TP_COMMON_H
#define _LINUX_INPUT_TP_COMMON_H

#include <linux/kobject.h>

struct tp_common_ops {
	ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf);
	ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count);
};

int tp_common_set_capacitive_keys_ops(const struct tp_common_ops *ops);
int tp_common_set_double_tap_ops(const struct tp_common_ops *ops);
int tp_common_set_reversed_keys_ops(const struct tp_common_ops *ops);
#ifdef CONFIG_PAPER_OPLUS_BACKLIGHT
int tp_common_set_oplus_backlight_ops(const struct tp_common_ops *ops);
void tp_common_remove_oplus_backlight_ops(void);
#endif
#endif
