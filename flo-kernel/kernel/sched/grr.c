#include "sched.h"

/*
 * grr-task scheduling class.
 *
 * (NOTE: these are not related to SCHED_grr tasks which are
 *  handled in sched_fair.c)
 */

#ifdef CONFIG_SMP
/*
* Wendan Kang: select cpu and balance. Cite from kernel/sched/fair.c
*/
static int
select_task_rq_grr(struct task_struct *p, int sd_flag, int flags)
{
	struct sched_domain *tmp, *affine_sd = NULL, *sd = NULL;
	int cpu = smp_processor_id();
	int prev_cpu = task_cpu(p);
	int new_cpu = cpu;
	int want_affine = 0;
	int want_sd = 1;
	int sync = wake_flags & WF_SYNC;

	if (p->rt.nr_cpus_allowed == 1)
		return prev_cpu;

	if (sd_flag & SD_BALANCE_WAKE) {
		if (cpumask_test_cpu(cpu, tsk_cpus_allowed(p)))
			want_affine = 1;
		new_cpu = prev_cpu;
	}

	rcu_read_lock();
	for_each_domain(cpu, tmp) {
		if (!(tmp->flags & SD_LOAD_BALANCE))
			continue;

		/*
		 * If power savings logic is enabled for a domain, see if we
		 * are not overloaded, if so, don't balance wider.
		 */
		if (tmp->flags & (SD_POWERSAVINGS_BALANCE|SD_PREFER_LOCAL)) {
			unsigned long power = 0;
			unsigned long nr_running = 0;
			unsigned long capacity;
			int i;

			for_each_cpu(i, sched_domain_span(tmp)) {
				power += power_of(i);
				nr_running += cpu_rq(i)->cfs.nr_running;
			}

			capacity = DIV_ROUND_CLOSEST(power, SCHED_POWER_SCALE);

			if (tmp->flags & SD_POWERSAVINGS_BALANCE)
				nr_running /= 2;

			if (nr_running < capacity)
				want_sd = 0;
		}

		/*
		 * If both cpu and prev_cpu are part of this domain,
		 * cpu is a valid SD_WAKE_AFFINE target.
		 */
		if (want_affine && (tmp->flags & SD_WAKE_AFFINE) &&
		    cpumask_test_cpu(prev_cpu, sched_domain_span(tmp))) {
			affine_sd = tmp;
			want_affine = 0;
		}

		if (!want_sd && !want_affine)
			break;

		if (!(tmp->flags & sd_flag))
			continue;

		if (want_sd)
			sd = tmp;
	}

	if (affine_sd) {
		if (cpu == prev_cpu || wake_affine(affine_sd, p, sync))
			prev_cpu = cpu;

		new_cpu = select_idle_sibling(p, prev_cpu);
		goto unlock;
	}

	while (sd) {
		int load_idx = sd->forkexec_idx;
		struct sched_group *group;
		int weight;

		if (!(sd->flags & sd_flag)) {
			sd = sd->child;
			continue;
		}

		if (sd_flag & SD_BALANCE_WAKE)
			load_idx = sd->wake_idx;

		group = find_idlest_group(sd, p, cpu, load_idx);
		if (!group) {
			sd = sd->child;
			continue;
		}

		new_cpu = find_idlest_cpu(group, p, cpu);
		if (new_cpu == -1 || new_cpu == cpu) {
			/* Now try balancing at a lower domain level of cpu */
			sd = sd->child;
			continue;
		}

		/* Now try balancing at a lower domain level of new_cpu */
		cpu = new_cpu;
		weight = sd->span_weight;
		sd = NULL;
		for_each_domain(cpu, tmp) {
			if (weight <= tmp->span_weight)
				break;
			if (tmp->flags & sd_flag)
				sd = tmp;
		}
		/* while loop will break here if sd == NULL */
	}
unlock:
	rcu_read_unlock();

	return new_cpu;
}
#endif /* CONFIG_SMP */
/*
 * grr tasks are unconditionally rescheduled:
 */
static void check_preempt_curr_grr(struct rq *rq, struct task_struct *p, int flags)
{
	resched_task(rq->grr);
}

/*Wendan Kang*/
/*The parameter(s) may have two options considering fair.c and rt.c:
*1. struct rq *rq, struct grr_rq *grr_rq
*2. struct grr_rq *grr_rq
*/
static struct sched_grr_entity *pick_next_grr_entity(/*...*/)
{
	/*...*/
}

/*Wendan Kang*/
static struct task_struct *pick_next_task_grr(struct rq *rq)
{
	struct task_struct *p;
	struct grr_rq *grr_rq = &rq->grr;
	struct sched_grr_entity *grr_se;

	if (!grr_rq->grr_nr_running)
		return NULL;

	/*...*/

	do {
		grr_se = pick_next_grr_entity(grr_rq?);
		/*...*/
	} while (grr_rq);

	/*...*/

	return p;
}

/*Wendan Kang: After change this fuction, change the relatives in kernel/sched/sched.h line 1197(approx)*/
/*The parameter(s) may have two options considering fair.c and rt.c:
*1. struct rq *rq, struct grr_rq *grr_rq
*2. struct grr_rq *grr_rq
*/
void init_grr_rq(/*...*/)
{
	
}

/*Wendan Kang*/
static void
dequeue_task_grr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_grr_entity *grr_se = &p->grr;
	/*...*/
}

static void put_prev_task_grr(struct rq *rq, struct task_struct *prev)
{
}

static void task_tick_grr(struct rq *rq, struct task_struct *curr, int queued)
{
}

static void set_curr_task_grr(struct rq *rq)
{
}

static void switched_to_grr(struct rq *rq, struct task_struct *p)
{
	BUG();
}

static void
prio_changed_grr(struct rq *rq, struct task_struct *p, int oldprio)
{
	BUG();
}

static unsigned int get_rr_interval_grr(struct rq *rq, struct task_struct *task)
{
	return 0;
}

/*
 * Simple, special scheduling class for the per-CPU grr tasks:
 */
const struct sched_class grr_sched_class = {
	/* .next is NULL */
	/* no enqueue/yield_task for grr tasks */

	/* dequeue is not valid, we print a debug message there: */
	.dequeue_task		= dequeue_task_grr,

	.check_preempt_curr	= check_preempt_curr_grr,

	.pick_next_task		= pick_next_task_grr,
	.put_prev_task		= put_prev_task_grr,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_grr,
#endif

	.set_curr_task          = set_curr_task_grr,
	.task_tick		= task_tick_grr,

	.get_rr_interval	= get_rr_interval_grr,

	.prio_changed		= prio_changed_grr,
	.switched_to		= switched_to_grr,
};
