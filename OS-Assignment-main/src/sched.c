
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>

static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
int prio_slot[MAX_PRIO];
#endif

int queue_empty(void)
{
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if (!empty(&mlq_ready_queue[prio]))
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void)
{
#ifdef MLQ_SCHED
	int i;
	for (i = 0; i < MAX_PRIO; i++)
	{
		mlq_ready_queue[i].size = 0;
		prio_slot[i] = MAX_PRIO - i;
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/*
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */

void increase_waitTime() {
    for (int i = 0; i < MAX_PRIO; i++) {
        if (mlq_ready_queue[i].size > 0) {
            // printf("Queue %d has size: %d\n", i, mlq_ready_queue[i].size);
            for (int j = 0; j < mlq_ready_queue[i].size; j++){
                if (mlq_ready_queue[i].proc[j] != NULL){
                    struct pcb_t *proc = mlq_ready_queue[i].proc[j];
                    proc->wait_time++;
                    // printf("Proc in queue[%d] updated wait_time: %d\n", i, proc->wait_time);
                    boost_prio(proc);
                }
            }
        }
    }
}

void boost_prio(struct pcb_t *proc){
    if(proc->wait_time > WAIT_THRESHOLD - 1 && proc->count_boost < MAX_BOOST){
        struct queue_t *cur_queue = &mlq_ready_queue[proc->prio];
        int found = 0;

        for(int i = 0; i < cur_queue->size; i++){
			pthread_mutex_lock(&queue_lock);
            if(cur_queue->proc[i] == proc) {
                // printf("Boosting process from queue[%d]\n", proc->prio);
                found = 1;
                for (int j = i; j < cur_queue->size - 1; j++) {
                    cur_queue->proc[j] = cur_queue->proc[j + 1];
                }
                cur_queue->size--;
				pthread_mutex_unlock(&queue_lock);
                break;
            }
			pthread_mutex_unlock(&queue_lock);
        }

        if(found){
            proc->wait_time = 0;
            proc->count_boost++;
            if(proc->prio > 0) {
                proc->prio--;
            }
			pthread_mutex_lock(&queue_lock);
            enqueue(&mlq_ready_queue[proc->prio], proc);
			pthread_mutex_unlock(&queue_lock);
            printf("Process %d boosted to priority: %d; waitTime: %d\n", proc->pid, proc->prio, proc->wait_time);
        }
    }
}


struct pcb_t *get_mlq_proc(void)
{
	struct pcb_t *proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */

	int cnt = 0;
	// static if non-preemptive
	int idx = 0;

	increase_waitTime();
	while (idx < MAX_PRIO)
	{

		if (prio_slot[idx] == 0)
		{
			// prio_slot[idx] = MAX_PRIO - idx;
		}
		else if (empty(&mlq_ready_queue[idx]))
		{
			++cnt;
		}
		else
		{	
			pthread_mutex_lock(&queue_lock);
			proc = dequeue(&mlq_ready_queue[idx]);
			pthread_mutex_unlock(&queue_lock);
			--prio_slot[idx];
			if(proc->count_boost > 0){
				if(proc->prio < proc->base_prio) proc->prio++;
				proc->count_boost--;
			}
			proc->wait_time = 0;
			break;
		}
		idx = (idx + 1) % MAX_PRIO;

		if (cnt == MAX_PRIO)
		{
			int flag = 0;
			for (int i = 0; i < MAX_PRIO; ++i)
			{
				if(!empty(&mlq_ready_queue[i]))
					flag = 1;
				prio_slot[i] = MAX_PRIO - i;
			}
			if (!flag)
				break;
		}
	}

	return proc;
}

void put_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	proc->wait_time = 0;
	proc->count_boost = 0;
	proc->base_prio = proc->prio;
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

struct pcb_t *get_proc(void)
{
	return get_mlq_proc();
}

void put_proc(struct pcb_t *proc)
{
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t *proc)
{
	return add_mlq_proc(proc);
}
#else
struct pcb_t *get_proc(void)
{
	struct pcb_t *proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	if (!empty(&ready_queue))
	{
		proc = dequeue(ready_queue);
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}
#endif
