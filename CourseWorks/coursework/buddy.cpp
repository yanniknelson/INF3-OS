/*
 * Buddy Page Allocation Algorithm
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (3)
 */

/*
 * STUDENT NUMBER: s
 */
#include <infos/mm/page-allocator.h>
#include <infos/mm/mm.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>
#include <infos/util/math.h>
#include <infos/util/printf.h>

using namespace infos::kernel;
using namespace infos::mm;
using namespace infos::util;

#define MAX_ORDER 17

/**
 * A buddy page allocation algorithm.
 */
class BuddyPageAllocator : public PageAllocatorAlgorithm
{
private:
	/**
	 * Returns the number of pages that comprise a 'block', in a given order.
	 * @param order The order to base the calculation off of.
	 * @return Returns the number of pages in a block, in the order.
	 */
	static inline constexpr uint64_t pages_per_block(int order)
	{
		/* The number of pages per block in a given order is simply 1, shifted left by the order number.
		 * For example, in order-2, there are (1 << 2) == 4 pages in each block.
		 */
		return (1 << order);
	}

	/**
	 * Returns TRUE if the supplied page descriptor is correctly aligned for the 
	 * given order.  Returns FALSE otherwise.
	 * @param pgd The page descriptor to test alignment for.
	 * @param order The order to use for calculations.
	 */
	static inline bool is_correct_alignment_for_order(const PageDescriptor *pgd, int order)
	{
		// Calculate the page-frame-number for the page descriptor, and return TRUE if
		// it divides evenly into the number pages in a block of the given order.
		return (sys.mm().pgalloc().pgd_to_pfn(pgd) % pages_per_block(order)) == 0;
	}

	/** Given a page descriptor, and an order, returns the buddy PGD.  The buddy could either be
	 * to the left or the right of PGD, in the given order.
	 * @param pgd The page descriptor to find the buddy for.
	 * @param order The order in which the page descriptor lives.
	 * @return Returns the buddy of the given page descriptor, in the given order.
	 */
	PageDescriptor *buddy_of(PageDescriptor *pgd, int order)
	{
		// (1) Make sure 'order' is within range
		if (order >= MAX_ORDER)
		{
			return NULL;
		}

		// (2) Check to make sure that PGD is correctly aligned in the order
		if (!is_correct_alignment_for_order(pgd, order))
		{
			return NULL;
		}

		// (3) Calculate the page-frame-number of the buddy of this page.
		// * If the PFN is aligned to the next order, then the buddy is the next block in THIS order.
		// * If it's not aligned, then the buddy must be the previous block in THIS order.
		uint64_t buddy_pfn = is_correct_alignment_for_order(pgd, order + 1) ? sys.mm().pgalloc().pgd_to_pfn(pgd) + pages_per_block(order) : sys.mm().pgalloc().pgd_to_pfn(pgd) - pages_per_block(order);

		// (4) Return the page descriptor associated with the buddy page-frame-number.
		return sys.mm().pgalloc().pfn_to_pgd(buddy_pfn);
	}

	/**
	 * Inserts a block into the free list of the given order.  The block is inserted in ascending order.
	 * @param pgd The page descriptor of the block to insert.
	 * @param order The order in which to insert the block.
	 * @return Returns the slot (i.e. a pointer to the pointer that points to the block) that the block
	 * was inserted into.
	 */
	PageDescriptor **insert_block(PageDescriptor *pgd, int order)
	{
		// Starting from the _free_area array, find the slot in which the page descriptor
		// should be inserted.
		PageDescriptor **slot = &_free_areas[order];

		// Iterate whilst there is a slot, and whilst the page descriptor pointer is numerically
		// greater than what the slot is pointing to.
		while (*slot && pgd > *slot)
		{
			slot = &(*slot)->next_free;
		}

		// Insert the page descriptor into the linked list.
		pgd->next_free = *slot;
		*slot = pgd;

		// Return the insert point (i.e. slot)
		return slot;
	}

	/**
	 * Removes a block from the free list of the given order.  The block MUST be present in the free-list, otherwise
	 * the system will panic.
	 * @param pgd The page descriptor of the block to remove.
	 * @param order The order in which to remove the block from.
	 */
	void remove_block(PageDescriptor *pgd, int order)
	{
		// Starting from the _free_area array, iterate until the block has been located in the linked-list.
		PageDescriptor **slot = &_free_areas[order];
		while (*slot && pgd != *slot)
		{
			slot = &(*slot)->next_free;
		}

		// Make sure the block actually exists.  Panic the system if it does not.
		assert(*slot == pgd);

		// Remove the block from the free list.
		*slot = pgd->next_free;
		pgd->next_free = NULL;
	}

	/**
	 * Given a pointer to a block of free memory in the order "source_order", this function will
	 * split the block in half, and insert it into the order below.
	 * @param block_pointer A pointer to a pointer containing the beginning of a block of free memory.
	 * @param source_order The order in which the block of free memory exists.  Naturally,
	 * the split will insert the two new blocks into the order below.
	 * @return Returns the left-hand-side of the new block.
	 */
	PageDescriptor *split_block(PageDescriptor **block_pointer, int source_order)
	{
		// Make sure there is an incoming pointer.
		assert(*block_pointer);

		// Make sure the block_pointer is correctly aligned.
		assert(is_correct_alignment_for_order(*block_pointer, source_order));

		// TODO: Implement this function

		//Make sure the we aren't trying to split the smallest size
		assert(source_order > 0);

		//remove the block to be split from the free list
		remove_block(*block_pointer, source_order);
		//add the left half of the block to be split to the free list
		insert_block(*block_pointer, source_order - 1);
		//add the right half of the block to be split to the free list
		insert_block(*block_pointer + pages_per_block(source_order - 1), source_order - 1);

		//return a pointer to the left buddy
		return *block_pointer;
	}

	/**
	 * Takes a block in the given source order, and merges it (and it's buddy) into the next order.
	 * This function assumes both the source block and the buddy block are in the free list for the
	 * source order.  If they aren't this function will panic the system.
	 * @param block_pointer A pointer to a pointer containing a block in the pair to merge.
	 * @param source_order The order in which the pair of blocks live.
	 * @return Returns the new slot that points to the merged block.
	 */
	PageDescriptor **merge_block(PageDescriptor **block_pointer, int source_order)
	{
		assert(*block_pointer);

		// Make sure the area_pointer is correctly aligned.
		assert(is_correct_alignment_for_order(*block_pointer, source_order));

		// TODO: Implement this function

		//get the buddy of the block to be merged
		PageDescriptor *bud = buddy_of(*block_pointer, source_order);

		//remove both buddies from the free list
		remove_block(*block_pointer, source_order);
		remove_block(bud, source_order);

		//inserts the merged buddies into the free list (order will increase by one) with the smaller (left most) block address (the valid address)
		//we can't assume the passed in block would be the start of the merged buddy it could be the second half and therefore not have the valid block address
		return insert_block((*block_pointer < bud) ? *block_pointer : bud, source_order + 1);
	}

public:
	/**
	 * Constructs a new instance of the Buddy Page Allocator.
	 */
	BuddyPageAllocator()
	{
		// Iterate over each free area, and clear it.
		for (unsigned int i = 0; i < ARRAY_SIZE(_free_areas); i++)
		{
			_free_areas[i] = NULL;
		}
	}

	/**
	 * Allocates 2^order number of contiguous pages
	 * @param order The power of two, of the number of contiguous pages to allocate.
	 * @return Returns a pointer to the first page descriptor for the newly allocated page range, or NULL if
	 * allocation failed.
	 */
	PageDescriptor *alloc_pages(int order) override
	{
		//ensure the desired order of the request is valid, if not return NULL
		if (order > MAX_ORDER)
		{
			return NULL;
		}

		//search for the first block that could fit the request
		//store the difference in orders between found block and desired order starting assuming it's 0
		int order_difference = 0;
		//while there are no free blocks of the desired order in the free list
		while (_free_areas[order + order_difference] == NULL)
		{
			//move to the next order
			order_difference++;
			//if the next order is invald return NULL
			if (order + order_difference > MAX_ORDER)
			{
				return NULL;
			}
		}
		//if the while loop exits then a block has been found at an equal or larger order
		//get a reference to that slot
		PageDescriptor **pg = &_free_areas[order + order_difference];

		//while the difference between the desired order and actual order of the block is greater than 0
		//split the block, decriment the difference and get a reference to the left most slot of the new order
		while (order_difference > 0)
		{
			split_block(pg, order + order_difference);
			order_difference--;
			pg = &_free_areas[order + order_difference];
		}
		//once this while loop has exited the pg should point to a free slot of the desired size
		//we must then remove the slot from the free list and return a pointer to its first page descriptor
		remove_block(*pg, order);
		return *pg;
	}

	/**
	 * Frees 2^order contiguous pages.
	 * @param pgd A pointer to an array of page descriptors to be freed.
	 * @param order The power of two number of contiguous pages to free.
	 */
	void free_pages(PageDescriptor *pgd, int order) override
	{
		// Make sure that the incoming page descriptor is correctly aligned
		// for the order on which it is being freed, for example, it is
		// illegal to free page 1 in order-1.
		assert(is_correct_alignment_for_order(pgd, order));

		//add the block back into the free list
		insert_block(pgd, order);

		//find the returned block's buddy
		PageDescriptor *bud = buddy_of(pgd, order);
		//while the current order of the block is less than the max
		while (order < MAX_ORDER)
		{
			//run through the the linked list of free blocks in the correct order until either the end is reached or the block is found
			PageDescriptor **slot = &_free_areas[order];
			while (*slot && bud != *slot)
			{
				slot = &(*slot)->next_free;
			}
			//If the slot pointer is NULL then the search didn't find the buddy free and so the job is done so return out of the function
			if (slot == NULL)
			{
				return;
			}
			//otherwise we want to merge the current block and store the result, increase the current order to be correct
			//and find the new buddy to repeat the search.
			slot = merge_block(slot, order);
			order++;
			bud = buddy_of(*slot, order);
		}
	}

	/**
	 * Reserves a specific page, so that it cannot be allocated.
	 * @param pgd The page descriptor of the page to reserve.
	 * @return Returns TRUE if the reservation was successful, FALSE otherwise.
	 */
	bool reserve_page(PageDescriptor *pgd)
	{
		PageDescriptor *block;
		PageDescriptor **slot;
		int order;
		for (order = MAX_ORDER; order > 0; order--)
		{
			//run through the the linked list of free blocks in the correct order until either the end is reached or the block containing the desired pae is found
			slot = &_free_areas[order];
			while (*slot)
			{
				//if the block being looked at starts at or before the desired page and ends after it, then it contains the desired page and stop scrolling
				if (*slot <= pgd && (*slot + pages_per_block(order)) > pgd)
				{
					block = split_block(slot, order);
					break;
				}
				slot = &(*slot)->next_free;
			}
		}
		slot = &_free_areas[order];
		while (*slot && pgd != *slot)
		{
			slot = &(*slot)->next_free;
		}
		if (*slot)
		{
			remove_block(pgd, 0);
			return true;
		}
		return false;
	}

	/**
	 * Initialises the allocation algorithm.
	 * @return Returns TRUE if the algorithm was successfully initialised, FALSE otherwise.
	 */
	bool init(PageDescriptor *page_descriptors, uint64_t nr_page_descriptors) override
	{
		mm_log.messagef(LogLevel::DEBUG, "Buddy Allocator Initialising pd=%p, nr=0x%lx", page_descriptors, nr_page_descriptors);

		// TODO: Initialise the free area linked list for the maximum order
		// to initialise the allocation algorithm.

		//starting at the first page descriptor insert a max_order block at every possible interval
		int count = 0;
		int offset = 0;
		while (offset < nr_page_descriptors)
		{
			insert_block(page_descriptors + offset, MAX_ORDER);
			page_descriptors += pages_per_block(MAX_ORDER);
			count++;
		}
		//return true if the correct number of buddies have been created otherwise return false
		return (count == nr_page_descriptors / pages_per_block(MAX_ORDER));
	}

	/**
	 * Returns the friendly name of the allocation algorithm, for debugging and selection purposes.
	 */
	const char *name() const override { return "buddy"; }

	/**
	 * Dumps out the current state of the buddy system
	 */
	void dump_state() const override
	{
		// Print out a header, so we can find the output in the logs.
		mm_log.messagef(LogLevel::DEBUG, "BUDDY STATE:");

		// Iterate over each free area.
		for (unsigned int i = 0; i < ARRAY_SIZE(_free_areas); i++)
		{
			char buffer[256];
			snprintf(buffer, sizeof(buffer), "[%d] ", i);

			// Iterate over each block in the free area.
			PageDescriptor *pg = _free_areas[i];
			while (pg)
			{
				// Append the PFN of the free block to the output buffer.
				snprintf(buffer, sizeof(buffer), "%s%lx ", buffer, sys.mm().pgalloc().pgd_to_pfn(pg));
				pg = pg->next_free;
			}

			mm_log.messagef(LogLevel::DEBUG, "%s", buffer);
		}
	}

private:
	PageDescriptor *_free_areas[MAX_ORDER];
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

/*
 * Allocation algorithm registration framework
 */
RegisterPageAllocator(BuddyPageAllocator);
