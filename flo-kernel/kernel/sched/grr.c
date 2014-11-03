#include "sched.h"

/*
 * grr-task scheduling class.
 *
 * (NOTE: these are not related to SCHED_grr tasks which are
 *  handled in sched_fair.c)
 */

const struct sched_class grr_sched_class;

/**************************************************************
 * GRR operations on generic schedulable entities:
 */


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
* Called by init_sched
*/
void init_grr_rq(struct grr_rq *grr_rq)
{
	struct sched_grr_entity *grr_entity;
	grr_rq->nr_running = 0;
	grr_rq->size = 0;
	grr_rq->curr = NULL;

	raw_spin_lock_init(&(grr_rq->grr_rq_lock));

	/* Initialize the run queue list */
	grr_entity = &grr_rq->run_queue;
	INIT_LIST_HEAD(&grr_entity->run_list);

	grr_entity->task = NULL;
	grr_entity->time_slice = 100;
	grr_entity->time_left = 0;
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
