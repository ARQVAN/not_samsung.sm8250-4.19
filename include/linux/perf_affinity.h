/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PERF_AFFINITY_H
#define _LINUX_PERF_AFFINITY_H

#include <linux/cpumask.h>

struct task_struct;
#ifdef CONFIG_PERF_AFFINITY
extern bool perf_affinity_ready;
void kthread_set_perf_affinity(struct task_struct *task,
			       const struct cpumask *mask);
void kthread_clear_perf_affinity(struct task_struct *task);
int irq_set_perf_affinity(unsigned int irq, const struct cpumask *mask);
void irq_reapply_perf_affinity(void);
#else
static inline void kthread_set_perf_affinity(struct task_struct *task,
					    const struct cpumask *mask) { }
static inline void kthread_clear_perf_affinity(struct task_struct *task) { }
static inline int irq_set_perf_affinity(unsigned int irq,
					const struct cpumask *mask) { return 0; }
#endif
#endif
