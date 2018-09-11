/* Copyright 2009, 2010 Brendan Tauras */

/* ssd_block.cpp is part of FlashSim. */

/* FlashSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version. */

/* FlashSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License
 * along with FlashSim.  If not, see <http://www.gnu.org/licenses/>. */

/****************************************************************************/

/* Block class
 * Brendan Tauras 2009-10-26
 *
 * The block is the data storage hardware unit where erases are implemented.
 * Blocks maintain wear statistics for the FTL. */

#include "ssd.h"
#include <assert.h>
#include <functional>
#include <new>
#include <stdio.h>

using namespace ssd;

Block::Block(const Plane& parent, uint block_size, ulong erases_remaining,
             double erase_delay, long physical_address)
	: physical_address(physical_address)
	, pages_invalid(0U)
	, size(block_size)
	,

	  /* use a const pointer (Page * const data) to use as an array
	   * but like a reference, we cannot reseat the pointer */
	  data((Page*)malloc(block_size * sizeof(Page)))
	, parent(parent)
	, pages_valid(0)
	,

	  state(FREE)
	,

	  /* set erases remaining to BLOCK_ERASES to match Block constructor args
	   * in Plane class
	   * this is the cheap implementation but can change to pass through classes */
	  erases_remaining(erases_remaining)
	,
      max_erases(erases_remaining)
    ,

	  /* assume hardware created at time 0 and had an implied free erasure */
	  last_erase_time(0.0)
	, erase_delay(erase_delay)
	,

	  modification_time(-1)

{
	uint i;

	if (erase_delay < 0.0) {
		fprintf(stderr, "Block warning: %s: constructor received negative "
		        "erase delay value\n\tsetting erase delay to 0.0\n",
		        __func__);
		erase_delay = 0.0;
	}

	/* new cannot initialize an array with constructor args so
	 * 	malloc the array
	 * 	then use placement new to call the constructor for each element
	 * chose an array over container class so we don't have to rely on anything
	 * 	i.e. STL's std::vector */
	/* array allocated in initializer list:
	 * data = (Page *) malloc(size * sizeof(Page)); */
	if (data == NULL) {
		fprintf(stderr,
		        "Block error: %s: constructor unable to allocate Page data\n",
		        __func__);
		exit(MEM_ERR);
	}
	for (i = 0; i < size; i++) {
		(void)new (&data[i]) Page(*this, PAGE_READ_DELAY, PAGE_WRITE_DELAY);
	}

#ifndef NOT_USE_BLOCKMGR
	// Creates the active cost structure in the block manager.
	// It assumes that it is created lineary.
	Block_manager::instance()->cost_insert(this);
#endif
	return;
}

Block::~Block(void)
{
	assert(data != NULL);
	uint i;
	/* call destructor for each Page array element
	 * since we used malloc and placement new */
	for (i = 0; i < size; i++)
		data[i].~Page();
	free(data);
	return;
}

enum status
Block::read(Event& event) {
	assert(data != NULL);
	return data[event.get_address().page]._read(event);
}

enum status
Block::write(Event& event) {
	assert(data != NULL);
	enum status ret = data[event.get_address().page]._write(event);

#ifndef NO_NOOP
	if (event.get_noop() == false)
	{
#endif
		pages_valid++;
#ifndef NO_BLOCK_STATE
		state = ACTIVE;
#endif
		modification_time = event.get_start_time();

#ifndef NOT_USE_BLOCKMGR
		Block_manager::instance()->update_block(this);
#endif
#ifndef NO_NOOP
	}
#endif

	return ret;
}

enum status
Block::replace(Event& event) {
    invalidate_page(event.get_replace_address().page, event.get_start_time() + event.get_time_taken() );
	return SUCCESS;
}

/* updates Event time_taken
 * sets Page statuses to EMPTY
 * updates last_erase_time and erases_remaining
 * returns 1 for success, 0 for failure */
enum status
Block::_erase(Event& event) {
	assert(data != NULL && erase_delay >= 0.0);
	uint i;

	if (!event.get_noop())
	{
		if (erases_remaining < 1) {
			fprintf(
			    stderr,
			    "Block error: %s: No erases remaining when attempting to erase\n",
			    __func__);
			return FAILURE;
		}

		for (i = 0; i < size; i++) {
			// assert(data[i].get_state() == INVALID);
			data[i].set_state(EMPTY);
		}

		event.incr_time_taken(erase_delay);
		last_erase_time = event.get_start_time() + event.get_time_taken();
        erases_remaining--;
		pages_valid = 0;
		pages_invalid = 0;
		state = FREE;

#ifndef NOT_USE_BLOCKMGR
		Block_manager::instance()->update_block(this);
#endif
	}

	return SUCCESS;
}

enum status
Block::_erase_and_copy(
    Event& event, Address& copyBlockAddr, Block* copyBlockPtr,
    std::function<void(const ulong, const Address&)> modifyFTL,
    std::function<void(const ulong, const uint pageNr)> modifyFTLPage) {
#ifdef DEBUG
	assert(copyBlockPtr != this);
	assert(data != NULL && erase_delay >= 0.0);
	const uint prevalid1 = copyBlockPtr->get_pages_valid();
	const uint prevalid2 = get_pages_valid();
	const uint preValids = prevalid1 + prevalid2;
#endif

	if (erases_remaining < 1)
	{
		fprintf(
		    stderr,
		    "Block error: %s: No erases remaining when attempting to erase\n",
		    __func__);
		return FAILURE;
	}

	uint copyPage = 0;
	pages_valid = 0;
	pages_invalid = 0;
#ifndef NO_BLOCK_STATE
	state = FREE;
#endif

	// Read valid lpns
	std::vector<ulong> lpns;
	for (uint i = 0; i < size; i++)
	{
		if (data[i].get_state() == VALID) {
            data[i]._read(event); // Must read data to copy them to other/this block later
			lpns.push_back(data[i].get_logical_address());
		}
		data[i].set_state(EMPTY);
	}
	for (const ulong lpn : lpns)
	{
		if (copyBlockPtr->get_next_page(copyPage) == SUCCESS) {
			copyBlockPtr->data[copyPage]._write(event, lpn);
			copyBlockPtr->pages_valid++;
#ifndef NO_BLOCK_STATE
			copyBlockPtr->state = ACTIVE;
#endif
			copyBlockAddr.page = copyPage;
			modifyFTL(lpn, copyBlockAddr);
			/// TODO After modifyFTL so modifyFTL can still reference old state
			// data[i].set_state(EMPTY);
		} else {
			uint first_empty = 0;
			get_next_page(first_empty);
			data[first_empty]._write(event, lpn);
			pages_valid++;

#ifndef NO_BLOCK_STATE
			state = ACTIVE;
#endif
			// Same block, different page
			modifyFTLPage(lpn, first_empty);
		}
	}

	event.incr_time_taken(erase_delay);
	last_erase_time = event.get_start_time() + event.get_time_taken();
	erases_remaining--;

#ifndef NOT_USE_BLOCKMGR
	Block_manager::instance()->update_block(this);
#endif
#ifdef DEBUG
	const uint postvalid1 = copyBlockPtr->get_pages_valid();
	const uint postvalid2 = get_pages_valid();
	const uint postValids = postvalid1 + postvalid2;
	assert(preValids == postValids);
#endif
	return SUCCESS;
}

std::vector<ulong>
Block::_read_logical_addresses_and_data(Event& event, const Block* block)
{
	assert(block->data != NULL);
	std::vector<ulong> lpns;
	for (uint i = 0; i < block->size; i++) {
        if (block->data[i].get_state() == VALID) {
            block->data[i]._read(event);
			const ulong lpn = block->data[i].get_logical_address();
			lpns.push_back(lpn);
		}
	}
	assert(lpns.size() == block->get_pages_valid());
	return lpns;
}

enum status
Block::_swap(FtlParent* ftl, Event& event, const Address& block1,
             const Address& block2,
             std::function<void(const ulong, const Address&)> modifyFTL) {
	Block* block1Ptr = ftl->get_block(block1);
	assert(block1Ptr->data != NULL && block1Ptr->erase_delay >= 0.0);
	Block* block2Ptr = ftl->get_block(block2);
	assert(block2Ptr->data != NULL && block2Ptr->erase_delay >= 0.0);

	if (block1Ptr->erases_remaining < 1 or block2Ptr->erases_remaining < 1)
	{
		fprintf(
		    stderr,
		    "Block error: %s: No erases remaining when attempting to erase\n",
		    __func__);
		return FAILURE;
	}
	// Read original lpn from event since we will overwrite it
	const ulong origLpn = event.get_logical_address();
	const Address origAddress = event.get_address();

    /// Read contents from blocks
    std::vector<ulong> block1Contents = _read_logical_addresses_and_data(event, block1Ptr);
	const bool block1Hotness = block1Ptr->get_block_hotness();
    std::vector<ulong> block2Contents = _read_logical_addresses_and_data(event, block2Ptr);
	const bool block2Hotness = block2Ptr->get_block_hotness();

	/// Copy from block 1 to block 2
	block1Ptr->_erase(event);
	for (const ulong& lpn : block2Contents)
	{
		Address writeAddr(block1);
		block1Ptr->get_next_page(writeAddr);
		event.set_address(writeAddr);
		event.set_logical_address(lpn);
		if (block1Ptr->write(event) == SUCCESS) {
			modifyFTL(lpn, writeAddr);
		} else {
			fprintf(
			    stderr,
			    "Block error: %s: Write failed when attempting to swap blocks\n",
			    __func__);
			return FAILURE;
		}
	}
	block1Ptr->set_block_hotness(block2Hotness);
	assert(block1Ptr->get_pages_valid() == block2Contents.size());
#ifndef NOT_USE_BLOCKMGR
	Block_manager::instance()->update_block(block1Ptr);
#endif

	/// Copy from block 2 to block 1
	block2Ptr->_erase(event);
	for (const ulong& lpn : block1Contents)
	{
		Address writeAddr(block2);
		block2Ptr->get_next_page(writeAddr);
		event.set_address(writeAddr);
		event.set_logical_address(lpn);
		if (block2Ptr->write(event) == SUCCESS) {
			modifyFTL(lpn, writeAddr);
		} else {
			fprintf(
			    stderr,
			    "Block error: %s: Write failed when attempting to swap blocks\n",
			    __func__);
			return FAILURE;
		}
	}
	block2Ptr->set_block_hotness(block1Hotness);
	assert(block2Ptr->get_pages_valid() == block1Contents.size());
#ifndef NOT_USE_BLOCKMGR
	Block_manager::instance()->update_block(block2Ptr);
#endif

	// Copy back original event lpn
	event.set_logical_address(origLpn);
	event.set_address(origAddress);

	return SUCCESS;
}

const Plane&
Block::get_parent(void) const
{
	return parent;
}

ssd::uint
Block::get_pages_valid(void) const
{
	return pages_valid;
}

ssd::uint
Block::get_pages_invalid(void) const
{
	return pages_invalid;
}

ssd::uint
Block::get_pages_empty(void) const
{
	const uint empty = size - pages_invalid - pages_valid;
	assert(empty <= size);
	return empty;
}

enum block_state
Block::get_state(void) const {
	return state;
}

enum page_state
Block::get_state(uint page) const {
	assert(data != NULL && page < size);
	return data[page].get_state();
}

enum page_state
Block::get_state(const Address& address) const {
	assert(data != NULL && address.page < size && address.valid >= BLOCK);
	return data[address.page].get_state();
}

double
Block::get_last_erase_time(void) const
{
    return last_erase_time;
}

double Block::get_last_page_invalidate_time(void) const
{
    return last_page_invalidate_time;
}

ssd::ulong
Block::get_erase_count(void) const
{
    return max_erases - erases_remaining;
}

ssd::ulong
Block::get_erases_remaining(void) const
{
	return erases_remaining;
}

ssd::uint
Block::get_size(void) const
{
	return size;
}

void
Block::invalidate_page(uint page, const double time)
{
	assert(page < size);

	if (data[page].get_state() == INVALID)
		return;

	if (data[page].get_state() == VALID)
		pages_valid--;
	pages_invalid++;
	data[page].set_state(INVALID);
    last_page_invalidate_time = time;

#ifndef NOT_USE_BLOCKMGR
	Block_manager::instance()->update_block(this);
#endif

#ifndef NO_BLOCK_STATE
	/* update block state */
	if (pages_invalid >= size)
		state = INACTIVE;
	else if (pages_valid > 0 || pages_invalid > 0)
		state = ACTIVE;
	else
		state = FREE;
#endif
	return;
}

///@TODO Remove
///double
///Block::get_modification_time(void) const
///{
///	return modification_time;
///}

/* method to find the next usable (empty) page in this block
 * method is called by write and erase methods and in Plane::get_next_page() */
enum status
Block::get_next_page(Address& address) const {
	for (uint i = 0; i < size; i++)
	{
		if (data[i].get_state() == EMPTY) {
#ifndef NOT_USE_BLOCKMGR
			address.set_linear_address(
			    i + physical_address - physical_address % BLOCK_SIZE, PAGE);
#endif
			address.page = i;
			address.valid = PAGE;
			return SUCCESS;
		}
	}
	return FAILURE;
}

enum status
Block::get_next_page(uint& pageNr) const {
	for (uint i = 0; i < size; i++)
	{
		if (data[i].get_state() == EMPTY) {
#ifndef NOT_USE_BLOCKMGR
			address.set_linear_address(
			    i + physical_address - physical_address % BLOCK_SIZE, PAGE);
#endif
			pageNr = i;
			return SUCCESS;
		}
	}
	return FAILURE;
}

long
Block::get_physical_address(void) const
{
	return physical_address;
}

Page*
Block::get_page_pointer(const Address& addr)
{
	assert(addr.valid >= PAGE);
	assert(addr.page < BLOCK_SIZE);
	return data[addr.page].get_pointer();
}

Block*
Block::get_pointer(void)
{
	return this;
}

block_type
Block::get_block_type(void) const
{
	return this->btype;
}

void
Block::set_block_type(block_type value)
{
	this->btype = value;
}

void
Block::set_block_hotness(const bool isHot)
{
	this->isHot = isHot;
}

bool
Block::get_block_hotness() const
{
	return this->isHot;
}
