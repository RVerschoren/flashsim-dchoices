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

#include <new>
#include <assert.h>
#include <stdio.h>
#include <functional>
#include "ssd.h"

using namespace ssd;

Block::Block(const Plane &parent, uint block_size, ulong erases_remaining, double erase_delay, long physical_address):
    physical_address(physical_address),
    pages_invalid(0),
    size(block_size),

    /* use a const pointer (Page * const data) to use as an array
     * but like a reference, we cannot reseat the pointer */
    data((Page *) malloc(block_size * sizeof(Page))),
    parent(parent),
    pages_valid(0),

    state(FREE),

    /* set erases remaining to BLOCK_ERASES to match Block constructor args
     * in Plane class
     * this is the cheap implementation but can change to pass through classes */
    erases_remaining(erases_remaining),

    /* assume hardware created at time 0 and had an implied free erasure */
    last_erase_time(0.0),
    erase_delay(erase_delay),

    modification_time(-1),

    min_seek_empty(0)

{
    uint i;

    if(erase_delay < 0.0)
    {
        fprintf(stderr, "Block warning: %s: constructor received negative erase delay value\n\tsetting erase delay to 0.0\n", __func__);
        erase_delay = 0.0;
    }

    /* new cannot initialize an array with constructor args so
     * 	malloc the array
     * 	then use placement new to call the constructor for each element
     * chose an array over container class so we don't have to rely on anything
     * 	i.e. STL's std::vector */
    /* array allocated in initializer list:
     * data = (Page *) malloc(size * sizeof(Page)); */
    if(data == NULL){
        fprintf(stderr, "Block error: %s: constructor unable to allocate Page data\n", __func__);
        exit(MEM_ERR);
    }
    for(i = 0; i < size; i++){
        (void) new (&data[i]) Page(*this, PAGE_READ_DELAY, PAGE_WRITE_DELAY);
    }

    // Creates the active cost structure in the block manager.
    // It assumes that it is created lineary.
    if(FTL_USE_BLOCKMANAGER) Block_manager::instance()->cost_insert(this);

    return;
}

Block::~Block(void)
{
    assert(data != NULL);
    uint i;
    /* call destructor for each Page array element
     * since we used malloc and placement new */
    for(i = 0; i < size; i++)
        data[i].~Page();
    free(data);
    return;
}

enum status Block::read(Event &event)
{
    assert(data != NULL);
    return data[event.get_address().page]._read(event);
}

enum status Block::write(Event &event)
{
    assert(data != NULL);
    enum status ret = data[event.get_address().page]._write(event);

    #ifndef NO_NOOP
    if(event.get_noop() == false)
    {
    #endif
        pages_valid++;
        #ifndef NO_BLOCK_STATE
        state = ACTIVE;
        #endif
        modification_time = event.get_start_time();

        #ifndef NOT_USE_BLOCKMGR
        if(FTL_USE_BLOCKMANAGER) Block_manager::instance()->update_block(this);
        #endif
    #ifndef NO_NOOP
    }
    #endif

    return ret;
}

enum status Block::replace(Event &event)
{
    invalidate_page(event.get_replace_address().page);
    return SUCCESS;
}

/* updates Event time_taken
 * sets Page statuses to EMPTY
 * updates last_erase_time and erases_remaining
 * returns 1 for success, 0 for failure */
enum status Block::_erase(Event &event)
{
    assert(data != NULL && erase_delay >= 0.0);
    uint i;

    if (!event.get_noop())
    {
        if(erases_remaining < 1)
        {
            fprintf(stderr, "Block error: %s: No erases remaining when attempting to erase\n", __func__);
            return FAILURE;
        }

        for(i = 0; i < size; i++)
        {
            //assert(data[i].get_state() == INVALID);
            data[i].set_state(EMPTY);
        }


        event.incr_time_taken(erase_delay);
        last_erase_time = event.get_start_time() + event.get_time_taken();
        erases_remaining--;
        pages_valid = 0;
        pages_invalid = 0;
        state = FREE;
        min_seek_empty = 0;

        if(FTL_USE_BLOCKMANAGER) Block_manager::instance()->update_block(this);
    }

    return SUCCESS;
}

enum status Block::_erase_and_copy(Event &event, Address &copyBlock, Block *copyBlockPtr, std::function<void (const ulong, const Address&)> modifyFTL, std::function<void (const ulong, const uint pageNr)> modifyFTLPage)
{
    assert(data != NULL && erase_delay >= 0.0);
    uint i;

    if(erases_remaining < 1)
    {
        fprintf(stderr, "Block error: %s: No erases remaining when attempting to erase\n", __func__);
        return FAILURE;
    }

    uint copyPage = 0;
    min_seek_empty = 0;
    pages_valid = 0;
    pages_invalid = 0;
    #ifndef NO_BLOCK_STATE
    state = FREE;
    #endif
    for(i = 0; i < size; i++)
    {
        if(data[i].get_state() == VALID)
        {
            const ulong lpn = data[i].get_logical_address();
            if(copyBlockPtr->get_next_page(copyPage) == SUCCESS)
            {
                copyBlockPtr->data[copyPage]._write(event, lpn);
                copyBlockPtr->pages_valid++;
                copyBlockPtr->state = ACTIVE;

                copyBlock.page = copyPage;
                modifyFTL(lpn,copyBlock);

                data[i].set_state(EMPTY);
            }else{
                data[min_seek_empty]._write(event, data[i].get_logical_address());
                pages_valid++;

                #ifndef NO_BLOCK_STATE
                state = ACTIVE;
                #endif

                if(i != min_seek_empty) data[i].set_state(EMPTY); //Could be we just copy page by page

                modifyFTLPage(lpn,min_seek_empty); //Same block, different page

                min_seek_empty++;
            }
        }else{
            data[i].set_state(EMPTY);
        }
    }

    event.incr_time_taken(erase_delay);
    last_erase_time = event.get_start_time() + event.get_time_taken();
    erases_remaining--;

    #ifndef NOT_USE_BLOCKMGR
        Block_manager::instance()->update_block(this);
    #endif

    return SUCCESS;
}


const Plane &Block::get_parent(void) const
{
    return parent;
}

ssd::uint Block::get_pages_valid(void) const
{
    return pages_valid;
}

ssd::uint Block::get_pages_invalid(void) const
{
    return pages_invalid;
}

ssd::uint Block::get_pages_empty(void) const
{
    const uint empty = size - pages_invalid - pages_valid;
    assert(empty <= size);
    return empty;
}


enum block_state Block::get_state(void) const
{
    return state;
}

enum page_state Block::get_state(uint page) const
{
    assert(data != NULL && page < size);
    return data[page].get_state();
}

enum page_state Block::get_state(const Address &address) const
{
   assert(data != NULL && address.page < size && address.valid >= BLOCK);
   return data[address.page].get_state();
}

double Block::get_last_erase_time(void) const
{
    return last_erase_time;
}

ssd::ulong Block::get_erases_remaining(void) const
{
    return erases_remaining;
}

ssd::uint Block::get_size(void) const
{
    return size;
}

void Block::invalidate_page(uint page)
{
    assert(page < size);

    if (data[page].get_state() == INVALID )
        return;

    if(data[page].get_state() == VALID) pages_valid--;
    pages_invalid++;
    data[page].set_state(INVALID);

    #ifndef NOT_USE_BLOCKMGR
    Block_manager::instance()->update_block(this);
    #endif

    #ifndef NO_BLOCK_STATE
    /* update block state */
    if(pages_invalid >= size)
        state = INACTIVE;
    else if(pages_valid > 0 || pages_invalid > 0)
        state = ACTIVE;
    else
        state = FREE;
    #endif
    return;
}

double Block::get_modification_time(void) const
{
    return modification_time;
}

/* method to find the next usable (empty) page in this block
 * method is called by write and erase methods and in Plane::get_next_page() */
enum status Block::get_next_page(Address &address) const
{
    for(uint i = min_seek_empty; i < size; i++)
    {
        if(data[i].get_state() == EMPTY)
        {
            #ifndef NOT_USE_BLOCKMGR
            address.set_linear_address(i + physical_address - physical_address % BLOCK_SIZE, PAGE);
            #endif
            address.page = i;
            address.valid = PAGE;
            const_cast<Block*>(this)->min_seek_empty  = i; //Shush away the constness in exchange for lookup speed in the future
            return SUCCESS;
        }
    }
    return FAILURE;
}

enum status Block::get_next_page(uint &pageNr) const
{
    for(uint i = min_seek_empty; i < size; i++)
    {
        if(data[i].get_state() == EMPTY)
        {
            #ifndef NOT_USE_BLOCKMGR
            address.set_linear_address(i + physical_address - physical_address % BLOCK_SIZE, PAGE);
            #endif
            pageNr = i;
            const_cast<Block*>(this)->min_seek_empty  = i; //Shush away the constness in exchange for lookup speed in the future
            return SUCCESS;
        }
    }
    return FAILURE;
}


long Block::get_physical_address(void) const
{
    return physical_address;
}

Page *Block::get_page_pointer(const Address &addr)
{
    assert(addr.valid >= PAGE);
    return data[addr.page].get_pointer();
}

Block *Block::get_pointer(void)
{
    return this;
}

block_type Block::get_block_type(void) const
{
    return this->btype;
}

void Block::set_block_type(block_type value)
{
    this->btype = value;
}

void Block::set_hotness(const bool isHot)
{
    this->isHot = isHot;
}

bool Block::get_hotness() const
{
    return this->isHot;
}

