/* Copyright 2020 Robin Verschoren */

/* hcwfwdiff_ftl.cpp  */

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

/* Implements a HCWF-based FTL. Keeps track of blocks in an erase count window. */

#include "ssd.h"
#include "util.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <vector>

using namespace ssd;

Addresses& FtlImpl_HCWFWDiff::get_erase_window(const ulong& pLinAddr, const uint W)
{
    const auto evtWmin = Wminmax.at(pLinAddr).first;
    const auto WIdx = WminIdx.at(pLinAddr) + (W - evtWmin);
#ifndef NDEBUG
    assert(WIdx < eraseWindows.at(pLinAddr).size());
#endif
    return eraseWindows.at(pLinAddr).at(WIdx);
}

FtlImpl_HCWFWDiff::FtlImpl_HCWFWDiff(Controller& controller, HotColdID* hcID, const uint dSwap, const uint Wdiff)
    : FtlImpl_HCWF(controller, hcID), dSwap(dSwap), eraseWindows{SSD_SIZE * PACKAGE_SIZE * DIE_SIZE},
      Wminmax{SSD_SIZE * PACKAGE_SIZE * DIE_SIZE, {0, Wdiff}}, WminIdx{SSD_SIZE * PACKAGE_SIZE * DIE_SIZE, 0}
{
}

FtlImpl_HCWFWDiff::~FtlImpl_HCWFWDiff() = default;

void FtlImpl_HCWFWDiff::initialize(const ulong numLPN)
{
    // Set up everything for HCWF itself
    FtlImpl_HCWF::initialize(numLPN);
    // Set up window
}

enum status FtlImpl_HCWFWDiff::_erase_swap_and_copy(Event& event, Address& victimBlock, Block* victimPtr,
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

void FtlImpl_HCWFWDiff::gca_collect(Event& event, Address& victim, const bool replacingHWF,
                                    const std::vector<Address>& doNotPick)
{
    const uint pLinAddr = PlaneAddress{victim}.to_linear_address();
    Addresses ignoreBlocks{get_erase_window(pLinAddr, Wminmax.at(pLinAddr).second)};
    std::copy(doNotPick.begin(), doNotPick.end(), std::back_inserter(ignoreBlocks));
    garbage->collect(event, victim, ignoreBlocks, replacingHWF);
}

void FtlImpl_HCWFWDiff::merge(Event& event, const Address& victimIn, const bool replacingHWF, Stats& stats)
{
    Address victim{victimIn};
	const uint j = get_pages_valid(victim);
	assert(j <= BLOCK_SIZE);
	Block* victimPtr = get_block(victim);
	const bool victimIsHot = get_block_hotness(victim);

    const bool victimHasSameHotness = (replacingHWF and victimIsHot) or not(replacingHWF or victimIsHot);

	PlaneAddress pAddr(victim);
    const uint pLinAddr = pAddr.to_linear_address();
    Address& evtHWF = HWF.at(pLinAddr);
    Address& evtCWF = CWF.at(pLinAddr);

	Address& sameWF = replacingHWF ? evtHWF : evtCWF;
	Address& otherWF = replacingHWF ? evtCWF : evtHWF;

    std::function<void(const ulong, const Address&)> modifyFTL = [this](const ulong lpn, const Address& addr) {
        map[lpn] = addr;
    };
    std::function<void(const ulong, const uint)> modifyFTLPage = [this](const ulong lpn, const uint newPage) {
        map[lpn].page = newPage;
    };

    if (get_block_erase_count(victim) == (Wminmax.at(pLinAddr).second - 1)) {
        // Choose swap block
        Address swapBlock = choose_swap_block(victim, replacingHWF);
        // Simple swap
        _erase_swap_and_copy(event, victim, victimPtr, victim, victimPtr, swapBlock, modifyFTL, replacingHWF, stats);
        // Swap with copying to otherWF
        //_erase_swap_and_copy(event, victim, victimPtr, otherWF, get_block(otherWF), swapBlock, modifyFTL,
        // replacingHWF,
        //                     stats);
        victim = swapBlock;
#ifndef NDEBUG
        check_ftl_hotness_integrity(event);
        check_block_hotness();
#endif
    }

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
        victimPtr->_erase_and_copy(event, otherWF,
                                   get_block(otherWF), // Copy to other WF where possible
                                   modifyFTL, modifyFTLPage);
        otherWF = victim;
        set_block_hotness(otherWF, !replacingHWF);
	}
    stats.erase_block(j, victimPtr->get_erases_remaining(), victimIsHot, victimIsHot ? j : 0, numHotBlocks);
}

Address FtlImpl_HCWFWDiff::choose_swap_block(const Address& victim, bool /*replacingHWF*/)
{
    const PlaneAddress pAddr(victim);
    const auto pLinAddr = pAddr.to_linear_address();
    const uint evtWmin = Wminmax.at(pLinAddr).first;
    const Addresses& WminAddrs = get_erase_window(pLinAddr, evtWmin);
    const uint numWmin = WminAddrs.size();
    const PlaneAddress eventPlaneAddr(victim);
    // std::vector<Address> doNotPick = {evtHWFAddr, evtCWFAddr, victim};

    std::function<bool(const Address&)> ignorePred = [&victim](const Address& candidate) {
        // Same block address, maybe even same page
        return victim.compare(candidate) >= BLOCK;
        // return block_is_in_vector(candidate, doNotPick);
    };

    uint maxValidPages = std::numeric_limits<uint>::min();
    uint maxValidIdx = 0;
    uint numMaxValidIdx = 0;

    if (dSwap == 0) { // Greedy
        for (uint idx = 0; idx < WminAddrs.size(); idx++) {
            const Address& candidate = WminAddrs.at(idx);
            if (ignorePred(candidate)) {
                continue;
            }
            const uint numValid = get_pages_valid(candidate);
            if (numValid > maxValidPages) {
                maxValidIdx = idx;
                maxValidPages = numValid;
                numMaxValidIdx = 1;
            } else if (numValid == maxValidPages) {
                numMaxValidIdx++;
                if (RandNrGen::get() < (1.0 / numMaxValidIdx)) {
                    maxValidIdx = idx;
                }
            }
        }
    } else {
        uint numTries = 0;
        while (numTries < dSwap) {
            const uint idx = RandNrGen::get(numWmin);
            const Address& candidate = WminAddrs.at(idx);
            if (ignorePred(candidate)) {
                continue;
            }
            numTries++;
            const uint numValid = get_pages_valid(candidate);
            if (numValid > maxValidPages) {
                maxValidIdx = idx;
                maxValidPages = numValid;
                numMaxValidIdx = 1;
            } else if (numValid == maxValidPages) {
                numMaxValidIdx++;
                if (RandNrGen::get() < (1.0 / numMaxValidIdx)) {
                    maxValidIdx = idx;
                }
            }
        }
    }
    return WminAddrs.at(maxValidIdx);
}
