/*
 * FIFO Scheduling Algorithm
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
 * A FIFO scheduling algorithm
 */
class FIFOScheduler : public SchedulingAlgorithm
{
public:
	/**
	 * Returns the friendly name of the algorithm, for debugging and selection purposes.
	 */
	const char *name() const override { return "fifo"; }

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
	 * e.g. its timeslice has not expired, or the algorithm determines it's not time to change.
	 */
	SchedulingEntity *pick_next_entity() override
	{
		//if there is nothing to run, return null to cause idle
		if (runqueue.empty())
		{
			return NULL;
		}
		//if the wait queue isn't empty get and return the first task without removing it from the queue
		return runqueue.first();
	}

	/*	
		I beleive the 'unresponsiveness' of /usr/sched-test2 using fifo is due two the test being comprised of 3 tasks
		Task 1, Task 2, and a task that listens for enter being pressed and then tells the other tasks to stop (Task 3). I think
		the tasks are added to the queue in the order I listed, meaning that fifo will run them (until they stop) in that order.
		This means Task 1 is run first until it stops, at which point Task 2 will run until it stops and then Task 3 will run.
		But Task 1 will only stop when Task 3 (which will only run after Task 1 AND Task 2 have both stopped) has run (and seen enter being pressed).
		i.e Task 1 and 2 need to be told to stop by Task 3 (the listener) but using fifo, Task 1 and 2 need to stop in order for Task 3 to tell them to stop.
	*/

private:
	// A list containing the current runqueue.
	List<SchedulingEntity *> runqueue;
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(FIFOScheduler);
