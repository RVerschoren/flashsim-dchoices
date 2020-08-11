/* Copyright 2018 Robin Verschoren */

/* hcwffalse_ftl.cpp  */

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

/* Implements a HCWF mechanism with possibly false hot/cold identification*/

#include "ssd.h"
#include "util.h"
#include <cassert>

using namespace ssd;

void FtlImpl_HCWFFixedFalse::initialize(const ulong numLPN)
{
    /// TODO  Review how to determine first cold block when not assuming 1 plane
    const double f = HOT_FRACTION;
    const uint numBlocks = PLANE_SIZE * DIE_SIZE * PACKAGE_SIZE * SSD_SIZE;
    maxHotBlocks = static_cast<uint>(std::ceil(f * numBlocks));
    const uint maxHotBlocksPerPlane = static_cast<uint>(std::ceil(f * PLANE_SIZE));
    const uint numColdBlocksPerPlane = PLANE_SIZE - maxHotBlocksPerPlane;

    numHotBlocks = 0;
    /// Set hotness
    for (uint package = 0; package < SSD_SIZE; package++) {
        for (uint die = 0; die < PACKAGE_SIZE; die++) {
            for (uint plane = 0; plane < DIE_SIZE; plane++) {
                PlaneAddress pAddr(package, die, plane);
                HWF[pAddr.to_linear_address()] = Address(package, die, plane, 0, 0, PAGE);
                CWF[pAddr.to_linear_address()] = Address(package, die, plane, maxHotBlocks, 0, PAGE);
                for (uint block = 0; block < PLANE_SIZE; block++) {
                    const Address blockAddr(package, die, plane, block, 0, BLOCK);
                    set_block_hotness(blockAddr, block < maxHotBlocksPerPlane);
                    if (block < maxHotBlocksPerPlane and numHotBlocks < maxHotBlocks) {
                        numHotBlocks++;
                    }
                }
            }
        }
    }
    // Set hotness for all hot WFs
    for (const auto& planeHWF : HWF) {
        set_block_hotness(planeHWF, true);
    }
    // Set hotness for all cold WFs
    for (const auto& planeCWF : CWF) {
        set_block_hotness(planeCWF, false);
    }

    assert(numHotBlocks <= maxHotBlocks);
    assert(numHotBlocks <= numBlocks);

    numHotLPN = static_cast<ulong>(std::floor(HOT_FRACTION * (numLPN - 1)));
    falseNegativeLimit = static_cast<ulong>(std::floor(prob_fneg * numHotLPN));
    falsePositiveLimit = numHotLPN + static_cast<ulong>(std::floor(prob_fpos * (numLPN - numHotLPN)));

    // Just assume for now that we have PAGE validity
    Address addr(0, 0, 0, 0, 0, PAGE);
    /// Initialize at random
    for (unsigned int lpn = 0; lpn < numLPN; lpn++) {
        bool success = false;
        const bool lpnIsHot = hcID->is_hot(lpn);
        const PlaneAddress pAddr = lpnToPlane(lpn);
        const ulong pLinAddr = pAddr.to_linear_address();
        addr.package = pAddr.package;
        addr.die = pAddr.die;
        addr.plane = pAddr.plane;
        while (not success) {
            if (lpnIsHot) {
                addr.block = RandNrGen::get(maxHotBlocksPerPlane);
            } else {
                addr.block = maxHotBlocksPerPlane + RandNrGen::get(numColdBlocksPerPlane);
            }
            assert(addr.check_valid() >= BLOCK);
            Block* block = get_block(addr);
            if (not HWF[pLinAddr].same_block(addr) and not CWF[pLinAddr].same_block(addr) and
                get_next_page(addr) == SUCCESS) {
                Event evt(WRITE, lpn, 1, 0);
                evt.set_address(addr);
                block->write(evt);
                map.push_back(addr);
                success = true;
            }
        }
    }
#ifndef NDEBUG
    check_valid_pages(numLPN);
    check_block_hotness();
    check_ftl_hotness_integrity(Event(event_type::READ, 0ul, 0, 0.0));
#endif
}

FtlImpl_HCWFFixedFalse::FtlImpl_HCWFFixedFalse(Controller& controller, HotColdID* hcID, double false_pos_prob,
                                               double false_neg_prob)
    : FtlImpl_HCWF(controller, hcID), prob_fpos(false_pos_prob), prob_fneg(false_neg_prob)
{
}

FtlImpl_HCWFFixedFalse::~FtlImpl_HCWFFixedFalse() = default;

enum status FtlImpl_HCWFFixedFalse::write(Event& event)
{
#ifndef NDEBUG
    check_valid_pages(map.size());
    check_block_hotness();
    check_ftl_hotness_integrity(event);
#endif
    const ulong lpn = event.get_logical_address();
    const PlaneAddress evtPlaneAddr = lpnToPlane(lpn);
    Address& evtHWF = HWF.at(evtPlaneAddr.to_linear_address());
    Address& evtCWF = CWF.at(evtPlaneAddr.to_linear_address());

    wlvl->prewrite(event, controller, {evtHWF, evtCWF}); /// WARNING Returns status, but no action needed on failure

    controller.stats.numFTLWrite++;

    /// Invalidate previous page
    get_block(map[lpn])->invalidate_page(map[lpn].page, event.get_start_time() + event.get_time_taken());

    /// FIXME This should be a FixedFalse HCID instead of FTL.
    const bool is_actually_hot = hcID->is_hot(lpn);
    const bool false_result = is_actually_hot ? (lpn < falseNegativeLimit) : (lpn < falsePositiveLimit);
    const bool lpnIsHot = false_result ? !is_actually_hot : is_actually_hot;

    while (get_next_page(evtHWF) != SUCCESS or get_next_page(evtCWF) != SUCCESS) // Still space in WF
    {
        const bool HWFInitiated = get_next_page(evtHWF) != SUCCESS; // HWF initiated cleaning cycle

        // Need to select a victim block through GC
        Address victim = map[lpn];
        const std::vector<Address> currentWF = {evtHWF, evtCWF};

        if (wlvl->suggest_WF(event, victim, controller, currentWF) == FAILURE) {
            this->gca_collect(event, victim, HWFInitiated, currentWF);
        }

        this->merge(event, victim, HWFInitiated, controller.stats);
    }
    if (lpnIsHot) {
        event.set_address(evtHWF); // Tell WF to write to this (next) page
        map[lpn] = evtHWF;
    } else {
        event.set_address(evtCWF); // Tell WF to write to this (next) page
        map[lpn] = evtCWF;
    }

#ifndef NDEBUG
    check_valid_pages(map.size() - 1);
#endif

    return SUCCESS;
}

void FtlImpl_HCWFFixedFalse::merge(Event& event, const Address& victim, const bool replacingHWF, Stats& stats)
{
    const uint j = get_pages_valid(victim);
    assert(j <= BLOCK_SIZE);
    // const uint k = HWFInitiated? CWFPtr->get_pages_empty() :
    // HWFPtr->get_pages_empty();
    // assert(k <= BLOCK_SIZE);
    Block* victimPtr = get_block(victim);
    const bool victimIsHot = get_block_hotness(victim);

    std::function<void(const ulong, const Address&)> modifyFTLFn = [this](const ulong lpn, const Address& newAddress) {
        this->modifyFTL(lpn, newAddress);
    };
    std::function<void(const ulong, const uint)> modifyFTLPageFn = [this](const ulong lpn, const uint newPage) {
        this->modifyFTLPage(lpn, newPage);
    };

    PlaneAddress evtPlaneAddr(victim);
    Address& evtHWF = HWF.at(evtPlaneAddr.to_linear_address());
    Address& evtCWF = CWF.at(evtPlaneAddr.to_linear_address());

    if (replacingHWF) {
        if (victimIsHot) {
            victimPtr->_erase_and_copy(event, evtHWF, get_block(evtHWF), // Copy to self
                                       modifyFTLFn, modifyFTLPageFn);
            evtHWF = victim;
        } else if (j <= get_block(evtCWF)->get_pages_empty()) {			 // Sufficient space to copy everything to CWF
            victimPtr->_erase_and_copy(event, evtCWF, get_block(evtCWF), // Copy to CWF where possible
                                       modifyFTLFn, modifyFTLPageFn);
            evtHWF = victim;
            set_block_hotness(evtHWF, true);
            numHotBlocks++;
        } else {
            victimPtr->_erase_and_copy(event, evtCWF, get_block(evtCWF), // Copy to CWF where possible
                                       modifyFTLFn, modifyFTLPageFn);
            evtCWF = victim;
            set_block_hotness(evtCWF, false);
        }

    } else { // CWF was full

        if (not victimIsHot) {
            victimPtr->_erase_and_copy(event, evtCWF, get_block(evtCWF), // Copy to self
                                       modifyFTLFn, modifyFTLPageFn);
            evtCWF = victim;
        } else if (j <= get_block(evtHWF)->get_pages_empty()) {			 // Sufficient space to copy everything to HWF
            victimPtr->_erase_and_copy(event, evtHWF, get_block(evtHWF), // Copy to HWF where possible
                                       modifyFTLFn, modifyFTLPageFn);
            evtCWF = victim;
            set_block_hotness(evtCWF, false);
            numHotBlocks--;
        } else {
            victimPtr->_erase_and_copy(event, evtHWF, get_block(evtHWF), // Copy to HWF where possible
                                       modifyFTLFn, modifyFTLPageFn);
            evtHWF = victim;
            set_block_hotness(evtHWF, true);
        }
    }
    /// TODO Should we do this for reporting too?
    uint hotPagesOnVictim = 0;
    Event tempEvent(READ, 0, 0, 0);
    const std::vector<ulong> victimLPNs = victimPtr->_read_logical_addresses_and_data(tempEvent);
    for (const ulong victimLPN : victimLPNs) {
        const bool is_actually_hot = hcID->is_hot(victimLPN);
        const bool false_result = is_actually_hot ? (victimLPN < falseNegativeLimit) : (victimLPN < falsePositiveLimit);
        const bool lpnIsHot = false_result ? !is_actually_hot : is_actually_hot;
        if (lpnIsHot) {
            hotPagesOnVictim++;
        }
    }

    stats.erase_block(j, victimPtr->get_erases_remaining(), victimIsHot, hotPagesOnVictim, numHotBlocks);
    ///@TODO Remove this
    /// if (controller.stats.get_currentPE() >=
    ///        victimPtr->get_erases_remaining()) {
    ///	controller.stats.next_currentPE();
    ///}
}
