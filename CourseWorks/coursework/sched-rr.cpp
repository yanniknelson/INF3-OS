/*
 * Round-robin Scheduling Algorithm
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (2)
 */

/*
 * STUDENT NUMBER: s
 */
#include <infos/kernel/sched.h>
#include <infos/kernel/thread.h>
#include <infos/kernel/log.h>
#include <infos/util/list.h>
#include <infos/util/lock.h>

using namespace infos::kernel;
using namespace infos::util;

/**
 * A round-robin scheduling algorithm
 */
class RoundRobinScheduler : public SchedulingAlgorithm
{
public:
	/**
	 * Returns the friendly name of the algorithm, for debugging and selection purposes.
	 */
	const char *name() const override { return "rr"; }

	/**
	 * Called when a scheduling entity becomes eligible for running.
	 * @param entity
	 */
	void add_to_runqueue(SchedulingEntity &entity) override
	{
		//ignore interrupts and add the entity to the end of the wait queue
		UniqueIRQLock l;
		runqueue.append(&entity);
	}

	/**
	 * Called when a scheduling entity is no longer eligible for running.
	 * @param entity
	 */
	void remove_from_runqueue(SchedulingEntity &entity) override
	{
		//ignore interrupts and remove the entity from the wait queue
		UniqueIRQLock l;
		runqueue.remove(&entity);
	}

	/**
	 * Called every time a scheduling event occurs, to cause the next eligible entity
	 * to be chosen.  The next eligible entity might actually be the same entity, if
	 * e.g. its timeslice has not expired.
	 */
	SchedulingEntity *pick_next_entity() override
	{
		//if there is nothing to run, return null to cause idle
		if (runqueue.empty())
		{
			return NULL;
		}
		//ignore interrupts as we will be making changes to the runqueue
		UniqueIRQLock l;
		//if the wait queue isn't empty get and remove the first task from the queue
		SchedulingEntity *next = runqueue.dequeue();
		//place that task at the end of the queue
		runqueue.append(next);
		//return that task to be run
		return next;

		//note that there is no need to check for the runqueue having one element and only returning runqueue.first, 
		//doing so would require a check every time slice which would only pays off if there is one element in the queue making it take more time overall.
		//whereas this method only performs an unnesscary operation if there is only one element in the runqueue (remove and then place back in).
	}

private:
	// A list containing the current runqueue.
	List<SchedulingEntity *> runqueue;
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(RoundRobinScheduler);
