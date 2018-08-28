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
Block_manager* Block_manager::inst = NULL;

FtlParent::FtlParent(Controller& controller)
	: controller(controller)
	, garbage(nullptr)
{
#ifndef NOT_USE_BLOCKMGR
	Block_manager::instance_initialize(this);
	printf("Number of addressable blocks: %u\n", NUMBER_OF_ADDRESSABLE_BLOCKS);
#else
	printf("Number of addressable blocks: %u\n", PLANE_SIZE);
#endif

	/// TODO Implement and enable remaining GCAs
	switch (GC_ALGORITHM) {
    case GC_FIFO:
		garbage = new GCImpl_FIFO(this);
		break;
    case GC_GREEDY:
		garbage = new GCImpl_Greedy(this);
		break;
    case GC_RANDOM:
		garbage = new GCImpl_Random(this);
		break;
    case GC_DCHOICES:
        garbage = new GCImpl_DChoices(this);
		break;
    case GC_COSTBENEFIT:
        garbage = new GCImpl_CostBenefit(this);
        break;
    case GC_COSTAGETIME:
        garbage = new GCImpl_CostAgeTime(this);
        break;
    case GC_DCHOICES_COSTBENEFIT:
        garbage = new GCImpl_DChoices_CostBenefit(this);
        break;
    case GC_DCHOICES_COSTAGETIME:
        garbage = new GCImpl_DChoices_CostAgeTime(this);
        break;
	default:
        garbage = new GCImpl_Greedy(this);
    }

	switch (WEAR_LEVELER) {
	case WL_NONE:
		wlvl = new Wear_leveler(this);
		break;
	case WL_BAN:
		wlvl = new WLvlImpl_Ban(this);
		break;
	case WL_BAN_PROB:
		wlvl = new WLvlImpl_Ban_Prob(this);
		break;
	case WL_MAXVALID:
		wlvl = new WLvlImpl_CleanMaxValid(this);
		break;
	case WL_RANDOMSWAP:
		wlvl = new WLvlImpl_RandomSwap(this);
		break;
	case WL_HOTCOLDSWAP:
		wlvl = new WLvlImpl_HotColdSwap(this);
		break;
	default:
		wlvl = new WLvlImpl_Ban(this);
	}
}

FtlParent::~FtlParent()
{
	if (garbage != nullptr) {
		delete garbage;
		garbage = nullptr;
	}
	if (wlvl != nullptr) {
		delete wlvl;
		wlvl = nullptr;
	}
}

void
FtlParent::initialize(const ulong /*numUniqueLPN*/)
{
	assert(false);
	return;
}

void
FtlParent::initialize(const std::set<ulong>& /*uniqueLPNs*/)
{
	assert(false);
	return;
}

ssd::ulong
FtlParent::get_erases_remaining(const Address& address) const
{
	return controller.get_erases_remaining(address);
}

void
FtlParent::get_least_worn(Address& address) const
{
	controller.get_least_worn(address);
	return;
}

enum page_state
FtlParent::get_state(const Address& address) const {
	return controller.get_state(address);
}

enum block_state
FtlParent::get_block_state(const Address& address) const {
	return controller.get_block_state(address);
}

Block*
FtlParent::get_block(const Address& address) const
{
	return controller.get_block(address);
}

void
FtlParent::cleanup_block(Event& /*event*/, Block* /*block*/)
{
	assert(false);
	return;
}

void
FtlParent::print_ftl_statistics()
{
	return;
}

uint
FtlParent::get_pages_valid(const Address& address) const
{
	assert(address.valid > NONE);
	return controller.get_pages_valid(address);
}

uint
FtlParent::get_pages_invalid(const Address& address) const
{
	assert(address.valid > NONE);
	return controller.get_pages_invalid(address);
}

uint
FtlParent::get_pages_erased(const Address& address) const
{
	assert(address.valid > NONE);
	return controller.get_pages_erased(address);
}

enum status
FtlParent::swap(Event& evt, const Address& block1, const Address& block2) {
	std::function<void(ulong, const Address&)> modifier =
	[this](ulong lpn, const Address& addr)
	{
		this->modifyFTL(lpn, addr);
	};
    return Block::_swap(this, evt, block1, block2, modifier);
}

void
FtlParent::modifyFTL(const ulong /*lpn*/, const Address& /*newAddress*/)
{
}

void
FtlParent::set_block_hotness(const Address& address, const bool hotness)
{
	assert(address.valid > NONE);
	controller.set_block_hotness(address, hotness);
	assert(get_block_hotness(address) == hotness);
}

bool
FtlParent::get_block_hotness(const Address& address) const
{
	assert(address.valid > NONE);
	return controller.get_block_hotness(address);
}

enum status
FtlParent::get_next_page(Address& address) const {
	assert(address.valid > NONE);
	return get_block(address)->get_next_page(address);
}
