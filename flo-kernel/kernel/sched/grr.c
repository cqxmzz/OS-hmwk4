#include "sched.h"

/*
 * grr-task scheduling class.
 *
 * (NOTE: these are not related to SCHED_grr tasks which are
 *  handled in sched_fair.c)
 */



/**************************************************************
 * GRR operations on generic schedulable entities:
 */
#ifdef CONFIG_GRR_GROUP_SCHED

/*Qiming Chen*/
#define for_each_sched_grr_entity(grr_se) \
	for (; grr_se; grr_se = grr_se->parent)

#define grr_entity_is_task(grr_se) (!(grr_se)->my_q)

static inline struct task_struct *grr_task_of(struct sched_grr_entity *grr_se)
{
#ifdef CONFIG_SCHED_DEBUG
	WARN_ON_ONCE(!grr_entity_is_task(grr_se));
#endif
	return container_of(grr_se, struct task_struct, grr);
}

/* runqueue on which this entity is (to be) queued */
static inline struct grr_rq *grr_rq_of_se(struct sched_grr_entity *grr_se)
{
	return grr_se->grr_rq;
}
static inline struct grr_rq *group_grr_rq(struct sched_grr_entity *grr_se)
{
	return grr_se->my_q;
}

/* Wendan Kang*/
void init_tg_grr_entry(struct task_group *tg, struct grr_rq *grr_rq,
			struct sched_grr_entity *grr_se, int cpu,
			struct sched_grr_entity *parent)
{
	struct rq *rq = cpu_rq(cpu);

	grr_rq->tg = tg;
	grr_rq->rq = rq;
#ifdef CONFIG_SMP
	/* allow initial update_cfs_load() to truncate */
	grr_rq->load_stamp = 1;
#endif
	tg->grr_rq[cpu] = grr_rq;
	tg->grr_se[cpu] = grr_se;

	/* se could be NULL for root_task_group */
	if (!grr_se)
		return;

	if (!parent)
		grr_se->grr_rq = &rq->grr;
	else
		grr_se->grr_rq = parent->my_q;

	grr_se->my_q = grr_rq;
	grr_se->parent = parent;
	INIT_LIST_HEAD(&grr_se->run_list);
}
/* Wendan Kang*/
void free_grr_sched_group(struct task_group *tg)
{
	int i;
	for_each_possible_cpu(i) {
		if (tg->grr_rq)
			kfree(tg->grr_rq[i]);
		if (tg->se)
			kfree(tg->grr_se[i]);
	}

	kfree(tg->grr_rq);
	kfree(tg->grr_se);
}
/* Wendan Kang*/
int alloc_grr_sched_group(struct task_group *tg, struct task_group *parent)
{
	struct grr_rq *grr_rq;
	struct sched_grr_entity *grr_se;
	int i;

	tg->grr_rq = kzalloc(sizeof(grr_rq) * nr_cpu_ids, GFP_KERNEL);
	if (!tg->grr_rq)
		goto err;
	tg->grr_se = kzalloc(sizeof(grr_se) * nr_cpu_ids, GFP_KERNEL);
	if (!tg->grr_se)
		goto err;


	for_each_possible_cpu(i) {
		grr_rq = kzalloc_node(sizeof(struct grr_rq),
				      GFP_KERNEL, cpu_to_node(i));
		if (!grr_rq)
			goto err;

		grr_se = kzalloc_node(sizeof(struct sched_entity),
				  GFP_KERNEL, cpu_to_node(i));
		if (!grr_se)
			goto err_free_rq;

		init_grr_rq(grr_rq);
		init_tg_grr_entry(tg, grr_rq, grr_se, i, parent->se[i]);
	}

	return 1;

err_free_rq:
	kfree(grr_rq);
err:
	return 0;
}
#else /* !CONFIG_GRR_GROUP_SCHED */

/*Qiming Chen*/
#define for_each_sched_grr_entity(rt_se) \
	for (; grr_se; grr_se = NULL)

#define grr_entity_is_task(grr_se) (1)


/* We probabily don't need this if we have the task pointer Qiming Chen*/
static inline struct task_struct *grr_task_of(struct sched_grr_entity *grr_se)
{
	return container_of(grr_se, struct task_struct, grr);
}

static inline struct grr_rq *grr_rq_of_se(struct sched_grr_entity *grr_se)
{
	struct task_struct *p = grr_task_of(grr_se);
	struct rq *rq = task_rq(p);

	return &rq->grr;
}
static inline struct grr_rq *group_grr_rq(struct sched_grr_entity *grr_se)
{
	return NULL;
}
void free_fair_sched_group(struct task_group *tg)
{
}
int alloc_grr_sched_group(struct task_group *tg, struct task_group *parent)
{
	return 1;
}
/* CONFIG_GRR_GROUP_SCHED */
#endif 

/***************************************************************
* Helping Methods
*/


/* Helper method that determines if the given entity is already
 * in the run queue. Return 1 if true and 0 if false */
static inline int on_grr_rq(struct sched_grr_entity *grr_se)
{
	/* If any item is on the grr_rq, then the run
	 * list will NOT be empty. */
	if (list_empty(&grr_se->run_list))
		return 0;
	else
		return 1;
}

/*
 * Update the current task's runtime statistics. Skip current tasks that
 * are not in our scheduling class.
 */
static void update_curr_grr(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	u64 delta_exec;

	if (curr->sched_class != &grr_sched_class)
		return;

	delta_exec = rq->clock_task - curr->se.exec_start;
	if (unlikely((s64)delta_exec < 0))
		delta_exec = 0;

	schedstat_set(curr->se.statistics.exec_max,
			max(curr->se.statistics.exec_max, delta_exec));

	curr->se.sum_exec_runtime += delta_exec;
	account_group_exec_runtime(curr, delta_exec);

	curr->se.exec_start = rq->clock_task;
	cpuacct_charge(curr, delta_exec);
}
/*
 * Put task to the head or the end of the run list without the overhead of
 * dequeue followed by enqueue.
 */
static void
requeue_grr_entity(struct grr_rq *grr_rq, struct sched_grr_entity *grr_se, int head)
{
	if (on_grr_rq(grr_se)) {
		struct list_head *queue;
		queue = &grr_rq->run_queue.run_list;

		if (head)
			list_move(&grr_se->run_list, queue);
		else
			list_move_tail(&grr_se->run_list, queue);
	}
}

static void requeue_task_grr(struct rq *rq, struct task_struct *p, int head)
{
	struct sched_grr_entity *grr_se = &p->grr;
	struct grr_rq *grr_rq;

	for_each_sched_grr_entity(grr_se) {
		grr_rq = grr_rq_of_se(grr_se);
		requeue_grr_entity(grr_rq, grr_se, head);
	}
}
/***************************************************************/

/***************************************************************
* load balance implementation
*/

#ifdef CONFIG_SMP
static void grr_rq_load_balance(void)
{
 	int cpu;
 	int dest_cpu; /* id of cpu to move to */
 	struct rq *rq;
 	struct grr_rq *lowest_grr_rq, *highest_grr_rq, *curr_grr_rq;
 	struct sched_grr_entity *heaviest_task_on_highest_grr_rq;
 	struct sched_grr_entity *curr_entity;
 	struct list_head *curr;
 	struct list_head *head;
 	struct task_struct *task_to_move;
 	struct rq *rq_of_task_to_move;
 	struct rq *rq_of_lowest_qrr; /*rq of thing with smallest weight */

 	int counter = 1;
 	int lowest_size = INT_MAX;
 	int highest_size = INT_MIN;

 	int largest_size = INT_MIN; /* used for load imblance issues*/

 	curr_entity = NULL;

 	for_each_online_cpu(cpu) {
 		rq = cpu_rq(cpu);
 		if (rq == NULL)
 			continue;

 		curr_grr_rq = &rq->grr;
 		if (curr_grr_rq->size > highest_size) {
 			highest_grr_rq = curr_grr_rq;
 			highest_size = curr_grr_rq->size;
 		}
 		if (curr_grr_rq->size < lowest_size) {
 			lowest_grr_rq = curr_grr_rq;
 			lowest_size = curr_grr_rq->size;
 		}
 		/* confirm if you don't have to do anything extra */
 		++counter;
 	}

 	if (lowest_grr_rq == highest_grr_rq)
 		return;
 	/* See if we can do move  */
 	/* Need to make sure that we don't cause a load imbalance */
 	head = &highest_grr_rq->run_queue.run_list;
 	spin_lock(&highest_grr_rq->grr_rq_lock);
 	for (curr = head->next; curr != head; curr = curr->next) {
 		curr_entity = list_entry(curr,
 					struct sched_grr_entity,
 					run_list);

 		if (curr_entity->size > largest_size) {
			heaviest_task_on_highest_wrr_rq = curr_entity;
 			largest_weight = curr_entity->weight;
 		}
 	}
 	spin_unlock(&highest_wrr_rq->wrr_rq_lock);

 	if (heaviest_task_on_highest_wrr_rq->weight +
 			lowest_wrr_rq->total_weight >=
 				highest_wrr_rq->total_weight)
 		/* there is an imbalance issues here */ {
 		return;
 	}
 	/* Okay, let's move the task */
 	rq_of_lowest_wrr = container_of(lowest_wrr_rq, struct rq, wrr);
 	dest_cpu = rq_of_lowest_wrr->cpu;
 	task_to_move = container_of(heaviest_task_on_highest_wrr_rq,
 				    struct task_struct, wrr);

 	rq_of_task_to_move = task_rq(task_to_move);
 	deactivate_task(rq_of_task_to_move, task_to_move, 0);

 	set_task_cpu(task_to_move, dest_cpu);
 	activate_task(rq_of_lowest_wrr , task_to_move, 0);
}
#endif

static enum hrtimer_restart print_current_time(struct hrtimer *timer)
{
        ktime_t period_ktime;
        struct timespec period = {
                .tv_nsec = SCHED_GRR_REBALANCE_TIME_PERIOD_NS,
                .tv_sec = 0
        };
        period_ktime = timespec_to_ktime(period);

        grr_rq_load_balance();

        hrtimer_forward(timer, timer->base->get_time(), period_ktime);
        return HRTIMER_RESTART;
}

/*Wendan Kang*/
/*The parameter(s) may have two options considering fair.c and rt.c:
*1. struct rq *rq, struct grr_rq *grr_rq
*2. struct grr_rq *grr_rq
*/
//static struct sched_grr_entity *pick_next_grr_entity(struct grr_rq * grr_rq)
//{
//	return NULL;
//}


/*Wendan Kang: After change this fuction, change the relatives in kernel/sched/sched.h line 1197(approx)*/
/*The parameter(s) may have two options considering fair.c and rt.c:
*1. struct rq *rq, struct grr_rq *grr_rq
*2. struct grr_rq *grr_rq
* Called by _schedinit
*/
void init_grr_rq(struct grr_rq *grr_rq)
{
	struct sched_grr_entity *grr_se;
	grr_rq->grr_nr_running = 0;
	grr_rq->size = 0;
	grr_rq->curr = NULL;

	spin_lock_init(&(grr_rq->grr_rq_lock));

	/* Initialize the run queue list */
	grr_se = &grr_rq->run_queue;
	INIT_LIST_HEAD(&grr_se->run_list);

	grr_se->task = NULL;
	grr_se->time_slice = 0;
	/* group? Qiming Chen */
}

/* Initializes the given task which is meant to be handled/processed
 * by this scheduler */
static void init_task_grr(struct task_struct *p)
{
	struct sched_grr_entity *grr_se;
	if (p == NULL)
		return;

	grr_se = &p->grr;
	grr_se->task = p;
	/*init time_slice, also as time left*/
	grr_se->time_slice = GRR_TIMESLICE;

	/* Initialize the list head just to be safe */
	INIT_LIST_HEAD(&grr_se->run_list);
	/* group? Qiming Chen*/
}

void init_sched_grr_class(void)
{
	/*don't know whether we need to implement that*/
}

/*Wendan Kang*/
static void
enqueue_task_grr(struct rq *rq, struct task_struct *p, int flags)
{
	struct list_head *head;
	struct sched_grr_entity *new_se;
	struct sched_grr_entity *grr_se;
	struct grr_rq *grr_rq = &rq->grr;

	grr_se = &grr_rq->run_queue;

	init_task_grr(p); /* initializes the grr_entity in task_struct */
	new_se = &p->grr;

	/* If on rq already, don't add it */
	if (on_grr_rq(new_se))
		return;

	spin_lock(&grr_rq->grr_rq_lock);

	/* add it to the queue.*/
	head = &grr_se->run_list;
	list_add_tail(&new_se->run_list, head);

	/* update statistics counts */
	++grr_rq->grr_nr_running;
	++grr_rq->size;

	spin_unlock(&grr_rq->grr_rq_lock);
}


/*Wendan Kang*/
static void
dequeue_task_grr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_grr_entity *grr_se = &p->grr;
	struct grr_rq *grr_rq = grr_rq_of_se(grr_se);

	spin_lock(&grr_rq->grr_rq_lock);

	update_curr_grr(rq);
	if (!on_grr_rq(grr_se)) { /* Should not happen */
		BUG();
		dump_stack();
	}

	/* Remove the task from the queue */
	list_del(&grr_se->run_list);

	/* update statistics counts */
	--grr_rq->grr_nr_running;
	--grr_rq->size;

	spin_unlock(&grr_rq->grr_rq_lock);
}

static void yield_task_grr(struct rq *rq)
{
	requeue_task_grr(rq, rq->curr, 0);
}

static struct sched_grr_entity *pick_next_grr_entity(struct rq *rq,
						   struct grr_rq *grr_rq)
{
	struct sched_grr_entity *first;
	struct sched_grr_entity *next = NULL;
	struct list_head *queue;

	first = &grr_rq->run_queue;
	queue = &first->run_list;
	next = list_entry(queue->next, struct sched_grr_entity, run_list);

	return next;
}
/*Wendan Kang*/
static struct task_struct *pick_next_task_grr(struct rq *rq)
{
	struct task_struct *p;
	struct grr_rq *grr_rq = &rq->grr;
	struct sched_grr_entity *grr_se;

	if (!grr_rq->grr_nr_running)
		return NULL;

	do {
		grr_se = pick_next_grr_entity(rq, grr_rq);
		BUG_ON(!grr_se);
		/* Wendan Kang: If grr_se contains a group, 
		*  need to find the first task in that group.
		*/
		grr_rq = group_grr_rq(grr_se);
	} while (grr_rq);

	p = grr_task_of(grr_se);
	p->se.exec_start = rq->clock_task;

	return p;
}

static void put_prev_task_grr(struct rq *rq, struct task_struct *prev)
{
	update_curr_grr(rq);
	/*Wendan Kang: there is more we can do here*/
}
static void set_curr_task_grr(struct rq *rq)
{
	struct task_struct *p = rq->curr;
	p->se.exec_start = rq->clock_task;
	rq->grr.curr = &p->grr;
}

static void task_tick_grr(struct rq *rq, struct task_struct *p, int queued)
{
	struct sched_grr_entity *grr_se = &p->grr;

	update_curr_grr(rq);

	/* watchdog(rq, p); */

	if (p->policy != SCHED_GRR)
		return;

	if (--p->grr.time_slice)
		return;

	p->grr.time_slice = GRR_TIMESLICE;

	/*
	 * Requeue to the end of queue if we (and all of our ancestors) are not the
	 * only element on the queue
	 */
	for_each_sched_grr_entity(grr_se) {
		if (grr_se->run_list.prev != grr_se->run_list.next) {
			requeue_task_grr(rq, p, 0);
			set_tsk_need_resched(p);
			return;
		}
	}
}

/* Wendan Kang
 * This function is called when a running process has changed its scheduler
 * and chosen to make this scheduler (GRR), its scheduler.
 * The ONLY THING WE NEED TO WORRRRRRRRRRRY ABOUT IS
 * load balance, whether make a task to switch to grr runqueue will cause
 * overload but as we implement load balance it's kind of like we don't need
 * do extra thing here.
 * UNLESS SOME CORNER CASEs REALLY HAPPEN T_T
 *
 * enqueue is called before switched_to_grr so we just set some default
 * value as init_task_grr did.
 * */
static void switched_to_grr(struct rq *rq, struct task_struct *p)
{
	if (!p->se.on_rq)
		return;

	init_task_grr(p);
}
/***************************************************************
* SMP Function below
*/
#ifdef CONFIG_SMP

static DEFINE_PER_CPU(cpumask_var_t, local_cpu_mask);
/* helper function: help to find cpu to assign task*/
static int find_lowest_rq(struct task_struct *task)
{
	struct sched_domain *sd;
	struct cpumask *lowest_mask = __get_cpu_var(local_cpu_mask);
	int this_cpu = smp_processor_id();
	int cpu      = task_cpu(task);

	/* Make sure the mask is initialized first */
	if (unlikely(!lowest_mask))
		return -1;

	if (task->rt.nr_cpus_allowed == 1)
		return -1; /* No other targets possible */

	if (!cpupri_find(&task_rq(task)->rd->cpupri, task, lowest_mask))
		return -1; /* No targets found */

	/*
	 * At this point we have built a mask of cpus representing the
	 * lowest priority tasks in the system.  Now we want to elect
	 * the best one based on our affinity and topology.
	 *
	 * We prioritize the last cpu that the task executed on since
	 * it is most likely cache-hot in that location.
	 */
	if (cpumask_test_cpu(cpu, lowest_mask))
		return cpu;

	/*
	 * Otherwise, we consult the sched_domains span maps to figure
	 * out which cpu is logically closest to our hot cache data.
	 */
	if (!cpumask_test_cpu(this_cpu, lowest_mask))
		this_cpu = -1; /* Skip this_cpu opt if not among lowest */

	rcu_read_lock();
	for_each_domain(cpu, sd) {
		if (sd->flags & SD_WAKE_AFFINE) {
			int best_cpu;

			/*
			 * "this_cpu" is cheaper to preempt than a
			 * remote processor.
			 */
			if (this_cpu != -1 &&
			    cpumask_test_cpu(this_cpu, sched_domain_span(sd))) {
				rcu_read_unlock();
				return this_cpu;
			}

			best_cpu = cpumask_first_and(lowest_mask,
						     sched_domain_span(sd));
			if (best_cpu < nr_cpu_ids) {
				rcu_read_unlock();
				return best_cpu;
			}
		}
	}
	rcu_read_unlock();

	/*
	 * And finally, if there were no matches within the domains
	 * just give the caller *something* to work with from the compatible
	 * locations.
	 */
	if (this_cpu != -1)
		return this_cpu;

	cpu = cpumask_any(lowest_mask);
	if (cpu < nr_cpu_ids)
		return cpu;
	return -1;
}

static int select_task_rq_grr(struct task_struct *p, int sd_flag, int flags)
{
	int cpu;
	int target;
	cpu = task_cpu(p);

	if (p->grr.nr_cpus_allowed == 1)
		goto out;

	/* For anything but wake ups, just return the task_cpu */
	if (sd_flag != SD_BALANCE_WAKE && sd_flag != SD_BALANCE_FORK)
		goto out;


	rcu_read_lock();
	target = find_lowest_rq(p);
	if (target != -1)
		cpu = target;
	rcu_read_unlock();

out:
	return cpu;
}

static void task_woken_grr(struct rq *rq, struct task_struct *p) {
	if (!p->se.on_rq)
		return;
	init_task_grr(p);
}

static unsigned int get_rr_interval_grr(struct rq *rq, struct task_struct *task) {
	if (task == NULL)
		return -EINVAL;
	return GRR_TIMESLICE;
}

static void set_cpus_allowed_grr(struct task_struct *p, const struct cpumask *new_mask) {
	/* Need implement Qiming Chen*/
}


/* leave then empty is OK */
static void check_preempt_curr_grr(struct rq *rq, struct task_struct *p, int flags) {
}

static void rq_online_grr(struct rq *rq) {
}

static void rq_offline_grr(struct rq *rq) {
}

static void pre_schedule_grr(struct rq *rq, struct task_struct *prev) {
}

static void post_schedule_grr(struct rq *rq) {
}

static void switched_from_grr(struct rq *rq, struct task_struct *p) {
}

static void prio_changed_grr(struct rq *rq, struct task_struct *p, int oldprio) {
}

#endif

/*
 * Simple, special scheduling class for the per-CPU grr tasks:
 */
const struct sched_class grr_sched_class = {
	.next 		= &fair_sched_class,              /*done*/
	.enqueue_task		= enqueue_task_grr,      /*done*/
	.dequeue_task		= dequeue_task_grr,      /*done*/
	.yield_task		= yield_task_grr,            /*done*/
	.check_preempt_curr	= check_preempt_curr_grr,/*done*/

	.pick_next_task		= pick_next_task_grr,    /*done*/
	.put_prev_task		= put_prev_task_grr,     /*done*/

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_grr,    /*done*/
	.set_cpus_allowed       = set_cpus_allowed_grr,
	.rq_online              = rq_online_grr,     /*dummy function*/
	.rq_offline             = rq_offline_grr,    /*dummy function*/
	.pre_schedule		= pre_schedule_grr,      /*dummy function*/
	.post_schedule		= post_schedule_grr,     /*dummy function*/
	.task_woken		= task_woken_grr,            /*dummy function*/
	.switched_from		= switched_from_grr,     /*dummy function*/
#endif

	.set_curr_task          = set_curr_task_grr, /*done*/
	.task_tick		= task_tick_grr,             /*done*/

	.get_rr_interval	= get_rr_interval_grr,   /*not sure what it does, haven't implement*/

	.prio_changed		= prio_changed_grr,      /*dummy function*/
	.switched_to		= switched_to_grr,       /*dummy function*/
};
