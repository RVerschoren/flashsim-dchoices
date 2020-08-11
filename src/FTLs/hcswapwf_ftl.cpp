/* Copyright 2019 Robin Verschoren */

/* hcswapwf_ftl.cpp  */

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

/* Implements a HCWF FTL with built-in swap during GCA. */

#include "ssd.h"
#include "util.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <vector>

using namespace ssd;

FtlImpl_HCSwapWF::FtlImpl_HCSwapWF(Controller& controller, HotColdID* hcID, double swap_probability, uint d)
	: FtlImpl_HCWF(controller, hcID), dSwap(d), pSwap(swap_probability)
{
}

FtlImpl_HCSwapWF::~FtlImpl_HCSwapWF() = default;

/*enum status FtlImpl_HCSwapWF::_erase_swap_and_copy(Event& event, Address& victimBlock, Block* victimPtr,
												   Address& copyBlock, Block* copyBlockPtr, Address& swapBlock,
												   std::function<void(const ulong, const Address&)> modifyFTL,
												   const bool replacingHWF, Stats& stats)
{

#ifndef NDEBUG
	assert(copyBlockPtr != victimPtr);
	assert(victimPtr->data != NULL && victimPtr->erase_delay >= 0.0);
	const uint prevalid1 = copyBlockPtr->get_pages_valid();
	const uint prevalid2 = victimPtr->get_pages_valid();
	uint preValids = prevalid1 + prevalid2;
	check_valid_pages(map.size() - 1);
	check_block_hotness();
	check_ftl_hotness_integrity();
#endif

	if (victimPtr->erases_remaining < 1) {
		fprintf(stderr, "Block error: %s: No erases remaining when attempting to erase\n", __func__);
		return FAILURE;
	}

	uint copyPage = 0;
#ifndef NO_BLOCK_STATE
	state = FREE;
#endif

	/// Choose swap block
	const bool victimBlockHotness = get_block_hotness(victimBlock);
	const PlaneAddress eventPlaneAddr(victimBlock);

	/// Get data from swap block
	Block* swapPtr = get_block(swapBlock);
#ifndef NDEBUG
	const uint prevalid3 = swapPtr->get_pages_valid();
	preValids += prevalid3;
	check_ftl_hotness_integrity();
#endif
	std::vector<ulong> swapLpns = swapPtr->_read_logical_addresses_and_data(event);
	const uint jswap = swapPtr->get_pages_valid();
	swapPtr->_erase(event);
	const bool swapBlockHotness = not victimBlockHotness;

	stats.erase_block(jswap, swapPtr->get_erases_remaining(), swapBlockHotness, swapBlockHotness ? jswap : 0,
					  numHotBlocks);

	/// Copy valid pages of victim block
	std::vector<ulong> lpns = victimPtr->_read_logical_addresses_and_data(event);
	victimPtr->_erase(event);
	for (const ulong lpn : lpns) {
		if (copyBlockPtr->get_next_page(copyPage) == SUCCESS) {
			copyBlockPtr->data[copyPage]._write(event, lpn);
			copyBlockPtr->pages_valid++;
#ifndef NO_BLOCK_STATE
			copyBlockPtr->state = ACTIVE;
#endif
			copyBlock.page = copyPage;
			modifyFTL(lpn, copyBlock);
		} else {

			uint first_empty = 0;
			swapPtr->get_next_page(first_empty);
			swapPtr->data[first_empty]._write(event, lpn);
			swapPtr->pages_valid++;

#ifndef NO_BLOCK_STATE
			swapPtr->state = ACTIVE;
#endif
			swapBlock.page = first_empty;
			modifyFTL(lpn, swapBlock);
#ifndef NDEBUG
			assert(hcID->is_hot(lpn) == victimBlockHotness);
			assert(swapBlockHotness != victimBlockHotness);
#endif
		}
	}
	set_block_hotness(swapBlock, victimBlockHotness);

	/// Copy everything from swap block to victim
	for (const ulong lpn : swapLpns) {
		uint victimPage = 0;
		victimPtr->get_next_page(victimPage);
		victimPtr->data[victimPage]._write(event, lpn);
		victimPtr->pages_valid++;
#ifndef NO_BLOCK_STATE
		victimPtr->state = ACTIVE;
#endif
		victimBlock.page = victimPage;
		modifyFTL(lpn, victimBlock);
#ifndef NDEBUG
		assert(hcID->is_hot(lpn) != victimBlockHotness);
#endif
	}
	set_block_hotness(victimBlock, not victimBlockHotness);

#ifndef NOT_USE_BLOCKMGR
	Block_manager::instance()->update_block(this);
#endif
#ifndef NDEBUG
	/// FIXME Review these postconditions
	const uint postvalid1 = copyBlockPtr->get_pages_valid();
	const uint postvalid2 = victimPtr->get_pages_valid();
	const uint postvalid3 = swapPtr->get_pages_valid();
	const uint postValids = postvalid1 + postvalid2 + postvalid3;
	assert(preValids == postValids);

	check_valid_pages(map.size() - 1);
	check_block_hotness();
	for (uint it = 0; it < map.size(); it++) {
		const ulong lpn = it;
		const Address& addr = map[it];
		if (lpn != event.get_logical_address())
			assert(get_block_hotness(addr) == hcID->is_hot(lpn));
	}
#endif
	/// FIXME Do this in stats
	stats.nFTLSwaps += 1;
	stats.FTLSwapCost[lpns.size() + swapLpns.size()] += 1;
	return SUCCESS;
}*/

enum status FtlImpl_HCSwapWF::_erase_swap_and_copy(Event& event, Address& victimBlock, Block* victimPtr,
												   Address& otherWF, Block* otherWFPtr, Address& swapBlock,
                                                   std::function<void(const ulong, const Address&)> modifyFTL,
                                                   const bool /*replacingHWF*/, Stats& stats)
{
	if (victimPtr->erases_remaining < 1) {
		fprintf(stderr, "Block error: %s: No erases remaining when attempting to erase\n", __func__);
		return FAILURE;
	}

#ifndef NO_BLOCK_STATE
	state = FREE;
#endif
#ifndef NDEBUG
    check_ftl_hotness_integrity(event);
#endif
	/// Choose swap block
	const bool victimBlockHotness = get_block_hotness(victimBlock);
	const PlaneAddress eventPlaneAddr(victimBlock);

	/// Get data from swap block
	Block* swapPtr = get_block(swapBlock);
	std::vector<ulong> swapLpns = swapPtr->_read_logical_addresses_and_data(event);
	const uint jswap = swapPtr->get_pages_valid();
	swapPtr->_erase(event);
    const bool swapBlockHotness = get_block_hotness(swapBlock);
	std::vector<ulong> lpns = victimPtr->_read_logical_addresses_and_data(event);
	victimPtr->_erase(event);
	stats.erase_block(jswap, swapPtr->get_erases_remaining(), swapBlockHotness, swapBlockHotness ? jswap : 0,
					  numHotBlocks);

	/// Copy valid pages of victim block
	uint copyPage = 0;
	uint first_empty = 0;
	for (const ulong lpn : lpns) {
		if (otherWFPtr->get_next_page(copyPage) == SUCCESS) {
			otherWFPtr->data[copyPage]._write(event, lpn);
			otherWFPtr->pages_valid++;
#ifndef NO_BLOCK_STATE
			otherWFPtr->state = ACTIVE;
#endif
			otherWF.page = copyPage;
			modifyFTL(lpn, otherWF);
		} else {
			swapPtr->get_next_page(first_empty);
			swapPtr->data[first_empty]._write(event, lpn);
			swapPtr->pages_valid++;
#ifndef NO_BLOCK_STATE
			swapPtr->state = ACTIVE;
#endif
			swapBlock.page = first_empty;
			modifyFTL(lpn, swapBlock);
		}
	}
	set_block_hotness(swapBlock, victimBlockHotness);

	/// Copy everything from swap block to victim
	for (const ulong lpn : swapLpns) {
		uint victimPage = 0;
		victimPtr->get_next_page(victimPage);
		victimPtr->data[victimPage]._write(event, lpn);
		victimPtr->pages_valid++;
#ifndef NO_BLOCK_STATE
		victimPtr->state = ACTIVE;
#endif
		victimBlock.page = victimPage;
		modifyFTL(lpn, victimBlock);
	}
    set_block_hotness(victimBlock, swapBlockHotness);
#ifndef NDEBUG
    check_ftl_hotness_integrity(event);
#endif

#ifndef NOT_USE_BLOCKMGR
	Block_manager::instance()->update_block(this);
#endif
	/// FIXME Do this in stats
	stats.nFTLSwaps += 1;
	stats.FTLSwapCost[lpns.size() + swapLpns.size()] += 1;
	return SUCCESS;
}

void FtlImpl_HCSwapWF::merge(Event& event, const Address& victim, const bool replacingHWF, Stats& stats)
{
	const uint j = get_pages_valid(victim);
	assert(j <= BLOCK_SIZE);
	Block* victimPtr = get_block(victim);
	const bool victimIsHot = get_block_hotness(victim);

	const bool victimHasSameHotness = (replacingHWF and victimIsHot) or not(replacingHWF or victimIsHot);

	PlaneAddress pAddr(victim);
	Address& evtHWF = HWF.at(pAddr.to_linear_address());
	Address& evtCWF = CWF.at(pAddr.to_linear_address());

	Address& sameWF = replacingHWF ? evtHWF : evtCWF;
	Address& otherWF = replacingHWF ? evtCWF : evtHWF;

	std::function<void(const ulong, const Address&)> modifyFTL = [this](const ulong lpn, const Address& addr) {
		map[lpn] = addr;
	};
    std::function<void(const ulong, const uint)> modifyFTLPage = [this](const ulong lpn, const uint newPage) {
        map[lpn].page = newPage;
    };

	if (victimHasSameHotness) {
		victimPtr->_erase_and_copy(event, sameWF, get_block(sameWF), // Copy to self
								   modifyFTL, modifyFTLPage);
		sameWF = victim;
	} else if (j <= get_block(otherWF)->get_pages_empty()) { // Sufficient space to copy everything to other WF
		victimPtr->_erase_and_copy(event, otherWF, get_block(otherWF), // Copy to other WF where possible
								   modifyFTL, modifyFTLPage);
		sameWF = victim;
		set_block_hotness(sameWF, replacingHWF);
		if (replacingHWF) {
			numHotBlocks++;
		} else {
			numHotBlocks--;
		}
#ifndef NDEBUG
		check_block_hotness();
#endif

	} else { // Victim cold for replacingHWF
		/// Choose swap block
		Address victimBlock(victim); // Bypass const
        Address swapBlock = choose_swap_block(victim, replacingHWF);
		/// Initiate swap if possible
		if (RandNrGen::get() < pSwap) { // and get_block_hotness(swapBlock) != victimIsHot) {
			_erase_swap_and_copy(event, victimBlock, victimPtr, otherWF, get_block(otherWF), swapBlock, modifyFTL,
								 replacingHWF, stats);
			// Previous otherHWF is full (because of copies from victim)
			otherWF = swapBlock;
            set_block_hotness(otherWF, !replacingHWF);
			// Replace sameWF with (swapped) victim
			sameWF = victimBlock;
            set_block_hotness(sameWF, replacingHWF);
#ifndef NDEBUG
            check_ftl_hotness_integrity(event);
            check_block_hotness();
#endif
		} else {
			victimPtr->_erase_and_copy(event, otherWF,
									   get_block(otherWF), // Copy to other WF where possible
									   modifyFTL, modifyFTLPage);
			otherWF = victim;
			set_block_hotness(otherWF, !replacingHWF);
		}
	}
	stats.erase_block(j, victimPtr->get_erases_remaining(), victimIsHot, victimIsHot ? j : 0, numHotBlocks);
}

Address FtlImpl_HCSwapWF::choose_swap_block(const Address& victim, bool /*replacingHWF*/)
{
	const bool victimHotness = get_block_hotness(victim);
	const PlaneAddress pAddr(victim);
	Address& evtHWF = HWF.at(pAddr.to_linear_address());
	Address& evtCWF = CWF.at(pAddr.to_linear_address());
	Address swapBlock(victim);
	const PlaneAddress eventPlaneAddr(victim);
	/// TODO There should be a better way for this: maybe extract a FtlImpl_HCWF::doNotPick-method
	/// returning all blocks which can not be used?
    std::vector<Address> doNotPick = {evtHWF, evtCWF, victim};
    std::function<uint(const Address&)> validPages = [this](const Address& addr) { return get_pages_valid(addr); };
    std::function<bool(const Address&)> ignorePred = [this, &doNotPick, victimHotness](const Address& candidate) {
		return get_block_hotness(candidate) == victimHotness or block_is_in_vector(candidate, doNotPick);
	};
	if (dSwap == 0) {
		// std::function<uint(const Address&)> costFunc = [this](const Address& addr) { return get_pages_valid(addr); };
		//		greedy_block_same_plane(swapBlock, costFunc,
		//		ignorePred);
        greedy_block_same_plane(swapBlock, validPages, ignorePred);
	} else {
        d_choices_block_same_plane(dSwap, swapBlock, validPages, ignorePred);
		// d_choices_block_same_plane(dSwap, swapBlock, costFunc, ignorePred);
	}
	return swapBlock;
}
