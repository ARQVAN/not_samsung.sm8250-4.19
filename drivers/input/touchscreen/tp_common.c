// SPDX-License-Identifier: GPL-2.0
/* Common touchpanel interface adapted from juezhi6668's Oplus backlight. */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/input/tp_common.h>
#include <linux/sysfs.h>

static struct kobject *touchpanel_kobj;

#define TS_ENABLE_FOPS(type) \
	static struct kobj_attribute type##_attr = \
		__ATTR(type, 0644, NULL, NULL); \
	int tp_common_set_##type##_ops(const struct tp_common_ops *ops) \
	{ \
		if (!touchpanel_kobj) \
			return -ENODEV; \
		if (!ops || !ops->show || !ops->store) \
			return -EINVAL; \
		type##_attr.show = ops->show; \
		type##_attr.store = ops->store; \
		return sysfs_create_file(touchpanel_kobj, &type##_attr.attr); \
	}

TS_ENABLE_FOPS(capacitive_keys)
TS_ENABLE_FOPS(double_tap)
TS_ENABLE_FOPS(reversed_keys)
#ifdef CONFIG_PAPER_OPLUS_BACKLIGHT
TS_ENABLE_FOPS(oplus_backlight)

void tp_common_remove_oplus_backlight_ops(void)
{
	if (touchpanel_kobj)
		sysfs_remove_file(touchpanel_kobj,
			&oplus_backlight_attr.attr);
}
#endif

static int __init tp_common_init(void)
{
	touchpanel_kobj = kobject_create_and_add("touchpanel", NULL);
	return touchpanel_kobj ? 0 : -ENOMEM;
}
core_initcall(tp_common_init);
