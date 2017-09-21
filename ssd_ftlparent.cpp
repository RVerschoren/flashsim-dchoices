/* Copyright 2011 Matias Bjørling */

/* ssd_ftlparent.cpp  */

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

/*
 * Implements parent interface for all FTL implementations to use.
 */

#include "ssd.h"

using namespace ssd;

// Initialization of the block layer.
Block_manager *Block_manager::inst = NULL;

FtlParent::FtlParent(Controller &controller) : controller(controller), garbage(nullptr)
{
    if(FTL_USE_BLOCKMANAGER) Block_manager::instance_initialize(this);

    printf("Number of addressable blocks: %u\n", NUMBER_OF_ADDRESSABLE_BLOCKS);

    ///@TODO Implement and enable remaining GCAs
    switch (GC_ALGORITHM)
    {
    //case 0:
    //	garbage = new GC_FIFO();
    //	break;
    //case 1:
    //	garbage = new GCImpl_Greedy(*this);
    //	break;
    case 2:
        garbage = new GCImpl_Random(this);
        break;
    case 3:
        garbage = new GCImpl_DChoices(this);
        break;
    default:
        garbage = new GCImpl_Random(this);
    }
}

FtlParent::~FtlParent()
{
    if(garbage != nullptr){
        delete garbage;
        garbage = nullptr;
    }
}

void FtlParent::initialize()
{
    assert(false);
    return;
}

ssd::ulong FtlParent::get_erases_remaining(const Address &address) const
{
    return controller.get_erases_remaining(address);
}

void FtlParent::get_least_worn(Address &address) const
{
    controller.get_least_worn(address);
    return;
}

enum page_state FtlParent::get_state(const Address &address) const
{
    return controller.get_state(address);
}

enum block_state FtlParent::get_block_state(const Address &address) const
{
    return controller.get_block_state(address);
}

Block *FtlParent::get_block_pointer(const Address &address)
{
    return controller.get_block_pointer(address);
}

void FtlParent::cleanup_block(Event &event, Block *block)
{
    assert(false);
    return;
}

void FtlParent::print_ftl_statistics()
{
    return;
}

uint FtlParent::get_pages_valid(const Address &address) const
{
    assert(address.valid > NONE);
    return controller.get_pages_valid(address);
}

uint FtlParent::get_pages_invalid(const Address &address) const
{
    assert(address.valid > NONE);
    return controller.get_pages_invalid(address);
}

uint FtlParent::get_pages_erased(const Address &address) const
{
    assert(address.valid > NONE);
    return controller.get_pages_erased(address);
}

