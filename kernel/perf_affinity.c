// SPDX-License-Identifier: GPL-2.0
/* Graphics cluster placement inspired by Sultan Alsawaf's tri-cluster API. */
#include <linux/cpu.h>
#include <linux/cpuhotplug.h>
#include <linux/init.h>
#include <linux/export.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/perf_affinity.h>
#include <linux/sched/task.h>
#include <linux/slab.h>

struct perf_kthread {
	struct list_head node;
	struct task_struct *task;
	const struct cpumask *mask;
};

bool perf_affinity_ready __read_mostly;
static LIST_HEAD(perf_kthreads);
static DEFINE_MUTEX(perf_kthreads_lock);

/* Only persistent workers which are eventually stopped may register. */
void kthread_set_perf_affinity(struct task_struct *task,
			       const struct cpumask *mask)
{
	struct perf_kthread *entry, *old;
	int ret;

	if (WARN_ON_ONCE(IS_ERR_OR_NULL(task) || !(task->flags & PF_KTHREAD) ||
	    (mask != cpu_perf_mask && mask != cpu_prime_mask)))
		return;
	if (!READ_ONCE(perf_affinity_ready))
		return;
	entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return;
	entry->task = task;
	entry->mask = mask;
	mutex_lock(&perf_kthreads_lock);
	list_for_each_entry(old, &perf_kthreads, node) {
		if (old->task == task) {
			mutex_unlock(&perf_kthreads_lock);
			kfree(entry);
			return;
		}
	}
	get_task_struct(task);
	list_add_tail(&entry->node, &perf_kthreads);
	/* Keep the full possible mask so ordinary hotplug can move the worker. */
	if (cpumask_intersects(mask, cpu_online_mask)) {
		ret = set_cpus_allowed_ptr(task, mask);
		if (ret)
			pr_warn_ratelimited("perf affinity: worker placement failed: %d\n", ret);
	}
	mutex_unlock(&perf_kthreads_lock);
}

EXPORT_SYMBOL_GPL(kthread_set_perf_affinity);

void kthread_clear_perf_affinity(struct task_struct *task)
{
	struct perf_kthread *entry;

	mutex_lock(&perf_kthreads_lock);
	list_for_each_entry(entry, &perf_kthreads, node) {
		if (entry->task == task) {
			list_del(&entry->node);
			mutex_unlock(&perf_kthreads_lock);
			put_task_struct(task);
			kfree(entry);
			return;
		}
	}
	mutex_unlock(&perf_kthreads_lock);
}

EXPORT_SYMBOL_GPL(kthread_clear_perf_affinity);

static int perf_affinity_online(unsigned int cpu)
{
	struct perf_kthread *entry;

	mutex_lock(&perf_kthreads_lock);
	list_for_each_entry(entry, &perf_kthreads, node) {
		if (cpumask_test_cpu(cpu, entry->mask))
			set_cpus_allowed_ptr(entry->task, entry->mask);
	}
	mutex_unlock(&perf_kthreads_lock);
	irq_reapply_perf_affinity();
	return 0;
}

static int __init perf_affinity_init(void)
{
	int ret = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "graphics:affinity",
				   perf_affinity_online, NULL);

	if (ret < 0)
		return ret;
	WRITE_ONCE(perf_affinity_ready, true);
	return 0;
}
core_initcall(perf_affinity_init);
