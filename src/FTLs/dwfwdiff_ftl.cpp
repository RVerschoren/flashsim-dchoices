/* Copyright 2020 Robin Verschoren */

/* dwfwdiff_ftl.cpp */

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

/* Implements a very simple page-level FTL without merge */

#include "ssd.h"
#include "util.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <vector>
using namespace ssd;

void check_block_address_wdiff(const Address& blockAddress)
{
    assert(blockAddress.package < SSD_SIZE);
    assert(blockAddress.die < PACKAGE_SIZE);
    assert(blockAddress.plane < DIE_SIZE);
    assert(blockAddress.block < PLANE_SIZE);
    assert(blockAddress.page < BLOCK_SIZE);
    assert(blockAddress.valid <= PAGE);
}

/* *******************************
 *  EraseWindow                  *
 *********************************/

FtlImpl_DWFWDiff::EraseWindow::EraseWindow(const uint Wdiff, const Plane& pl)
    : Wmin(0), Wmax(Wdiff), Wdiff(Wdiff), plane(const_cast<Plane&>(pl)), eraseWindow(pl.get_size()),
      WIdx(Wdiff + 1, pl.get_size()), blockNrToIdx(pl.get_size())
{
    WIdx.at(0) = 0;
    std::iota(eraseWindow.begin(), eraseWindow.end(), 0);
    std::iota(blockNrToIdx.begin(), blockNrToIdx.end(), 0);
}

Address FtlImpl_DWFWDiff::EraseWindow::choose_Wmin_maxValid_BlockNr(const Address& victim, uint dSwap)
{
#ifndef NDEBUG
	_check_window(victim);
#endif
	const uint numWmin = WIdx.at(1) - WIdx.at(0);

	uint maxValidPages = std::numeric_limits<uint>::min();
	uint maxValidIdx = 0;
	uint numMaxValidIdx = 0;

	Address candidate{victim};
	if (dSwap == 0) { // Greedy
		for (uint blockNr = 0; blockNr < numWmin; blockNr++) {
			candidate.block = blockNr;
			// Same block address, maybe even same page
			if (victim.compare(candidate) >= BLOCK) {
				continue;
			}
			const uint numValid = plane.get_block_pages_valid(candidate);
			if (numValid > maxValidPages) {
				maxValidIdx = blockNr;
				maxValidPages = numValid;
				numMaxValidIdx = 1;
			} else if (numValid == maxValidPages) {
				numMaxValidIdx++;
				if (RandNrGen::get() < (1.0 / numMaxValidIdx)) {
					maxValidIdx = blockNr;
				}
			}
		}
	} else {
		uint numTries = 0;
		while (numTries < dSwap) {
			const uint blockNr = RandNrGen::get(numWmin);
			candidate.block = blockNr;
			// Same block address, maybe even same page
			if (victim.compare(candidate) >= BLOCK) {
				continue;
			}
			numTries++;
			const uint numValid = plane.get_block_pages_valid(candidate);
			if (numValid > maxValidPages) {
				maxValidIdx = blockNr;
				maxValidPages = numValid;
				numMaxValidIdx = 1;
			} else if (numValid == maxValidPages) {
				numMaxValidIdx++;
				if (RandNrGen::get() < (1.0 / numMaxValidIdx)) {
					maxValidIdx = blockNr;
				}
			}
		}
	}
#ifndef NDEBUG
	_check_window(victim);
#endif
	candidate.block = maxValidIdx;
	return candidate;
}

void FtlImpl_DWFWDiff::EraseWindow::count_erase(const Address& eBlock)
{
#ifndef NDEBUG
    assert((Wmax - Wmin) <= Wdiff);
#endif
	const uint eBlockNr = eBlock.block;
	uint& eBlockIdx = blockNrToIdx.at(eBlockNr);

    uint ebWit = 0;
	for (uint i = 0; i < WIdx.size() - 1; i++) {
		if (WIdx.at(i) <= eBlockIdx && eBlockIdx < WIdx.at(i + 1)) {
			ebWit = i;
			break;
		}
    }

    // ebWit is W(eBlock) - Wmin
    uint nextWit = ebWit + 1;
    uint& nextWIdx = WIdx.at(nextWit);

    if (ebWit == 0 && nextWIdx == 1) { // Last of the Wmin
        // Don't need to modify or swap, just:
        // Slide window
        Wmin = Wmin + 1;
        Wmax = Wmax + 1;
        // Copy indexes to reflect this
        for (uint it = 1; it < WIdx.size(); it++) {
            WIdx.at(it - 1) = WIdx.at(it);
        }
        // Adjust last index
        WIdx.at(WIdx.size() - 1) = eraseWindow.size();
    } else { // Arbitrary block in window
        // Last block with erasures equal to eBlock
        const uint sBlockIdx = nextWIdx - 1;
        const uint sBlockNr = eraseWindow.at(sBlockIdx);
        // Swap them and keep track of indexes
        // First s and then e (as eBlockIdx is reference)
        eraseWindow.at(sBlockIdx) = eBlockNr;
        blockNrToIdx.at(sBlockNr) = eBlockIdx;
        eraseWindow.at(eBlockIdx) = sBlockNr;
        blockNrToIdx.at(eBlockNr) = sBlockIdx;
        // Expand next part of the window by 1 (towards the front)
        nextWIdx--;
    }
#ifndef NDEBUG
    assert((Wmax - Wmin) <= Wdiff);
    //_check_window(eBlock);
#endif
}

void FtlImpl_DWFWDiff::EraseWindow::_check_window(const Address& baseAddr, bool baseAddrIsUnerasedVictim)
{
	Address check{baseAddr};
    uint currentW = Wmin;
    uint nextWIt = 1;
	for (uint e = 0; e < eraseWindow.size(); e++) {
        if (nextWIt == WIdx.size()) {
            currentW = Wmax;
        } else if (e == WIdx.at(nextWIt)) {
            currentW++;
            nextWIt++;
        }
        const uint eBlock = eraseWindow.at(e);
        assert(blockNrToIdx.at(eBlock) == e);
        check.block = eBlock;
        const uint eraseCount = plane.get_block_erase_count(check);
        assert(eraseCount == currentW ||
               (baseAddrIsUnerasedVictim && baseAddr.block == eBlock && eraseCount == currentW - 1));
	}
}

Addresses FtlImpl_DWFWDiff::EraseWindow::get_block_addresses(const uint W, const Address& baseAddr) const
{
    const uint currentWIdx = WIdx.at(W - Wmin);
    const uint nextWIdx = (W < Wmax) ? WIdx.at(W - Wmin + 1) : eraseWindow.size();
    Addresses Wblocks{nextWIdx - currentWIdx + 1, baseAddr};
    for (uint idx = currentWIdx; idx < nextWIdx; idx++) {
        Wblocks.at(idx - currentWIdx).block = eraseWindow.at(idx);
    }
    return Wblocks;
}

/* *******************************
 *  FtlImpl_DWFWDiff             *
 *********************************/

void FtlImpl_DWFWDiff::initialize(const ulong numLPN)
{
	// Just assume for now that we have PAGE validity, we'll check it later
	// anyway

	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
                const Address planeAddr{package, die, plane, 0, 0, PLANE};
                Plane& pl = *(this->controller.get_plane_pointer(planeAddr));
				const PlaneAddress pAddr(package, die, plane);
				const ulong pLinAddr = pAddr.to_linear_address();
				WFE[pLinAddr] = Address(package, die, plane, 0, 0, PAGE);
				WFI[pLinAddr] = Address(package, die, plane, 1, 0, PAGE);
                eraseWindows.push_back(FtlImpl_DWFWDiff::EraseWindow{deltaW, pl});
			}
		}
	}
	Address addr(0, 0, 0, 0, 0, PAGE);
	for (ulong lpn = 0; lpn < numLPN; lpn++) {
		bool success = false;
		// const bool lpnIsHot = hcID.is_hot(lpn);
		const PlaneAddress pAddr = lpnToPlane(lpn);
		const ulong pLinAddr = pAddr.to_linear_address();
		addr.package = pAddr.package;
		addr.die = pAddr.die;
		addr.plane = pAddr.plane;
		while (not success) {
			random_block_same_plane(addr);
			assert(addr.check_valid() >= BLOCK);
			assert(addr.block < PLANE_SIZE);

			Block* block = get_block(addr);

			if (!WFE[pLinAddr].same_block(addr) && !WFI[pLinAddr].same_block(addr) &&
				block->get_next_page(addr) == SUCCESS) {
				Event evt(WRITE, lpn, 1, 0);
				evt.set_address(addr);
				block->write(evt);
				map.push_back(addr);
				///@TODO Enable hotvalidpages for DWF
				/// if (hcID->is_hot(lpn)) {
				/// hotValidPages[addr.package][addr.die][addr.plane][addr.block]++;
				/// }
				success = true;
			}
		}
	}
	std::cout << "INIT DONE" << std::endl;

#ifndef NDEBUG
    check_valid_pages();
	check_ftl_integrity();
	Address a(0, 0, 0, 0, 0, PAGE);
	for (uint i = 0; i < PLANE_SIZE; i++) {
		a.block = i;
		/// TODO check_hot_pages(a, get_block_pointer(a),
		/// hotValidPages[a.package][a.die][a.plane][a.block]);
	}
#endif
}

FtlImpl_DWFWDiff::FtlImpl_DWFWDiff(Controller& controller, HotColdID* hcID, const uint dSwap, const uint deltaW)
    : FtlImpl_DWF(controller, hcID), dStar(dSwap), deltaW(deltaW)
{
}

FtlImpl_DWFWDiff::~FtlImpl_DWFWDiff() = default;

void FtlImpl_DWFWDiff::modifyFTL(const ulong lpn, const Address& newAddr)
{
	assert(map.at(lpn).package < SSD_SIZE);
	assert(map.at(lpn).die < PACKAGE_SIZE);
	assert(map.at(lpn).block < PLANE_SIZE);
	assert(map.at(lpn).plane < DIE_SIZE);
	assert(map.at(lpn).page < BLOCK_SIZE);
	assert(map.at(lpn).valid <= PAGE);

	Address oldAddr = map.at(lpn);
	map[lpn] = newAddr;

	assert(newAddr.block < PLANE_SIZE);
	assert(map.at(lpn).block < PLANE_SIZE);
	assert(map.at(lpn).valid <= PAGE);
	const bool pageWasValid = get_state(oldAddr) == VALID;
	/// TODO hcID->is_hot(lpn) or false?
	const bool pageIsHot = hcID->is_hot(lpn); // false;
	if (pageWasValid and pageIsHot) {
		recompute_hotvalidpages(oldAddr);
		recompute_hotvalidpages(newAddr);
	}
	assert(map.at(lpn).package < SSD_SIZE);
	assert(map.at(lpn).die < PACKAGE_SIZE);
	assert(map.at(lpn).block < PLANE_SIZE);
	assert(map.at(lpn).plane < DIE_SIZE);
	assert(map.at(lpn).page < BLOCK_SIZE);
	assert(map.at(lpn).valid <= PAGE);
}

void FtlImpl_DWFWDiff::merge(Event& event, const Address& victim)
{
	Block* victimPtr = get_block(victim);
	PlaneAddress pAddr(victim);
    const auto pLinAddr = pAddr.to_linear_address();
    Address& evtWFE = WFE.at(pLinAddr);
    Address& evtWFI = WFI.at(pLinAddr);
    auto& evtWindow = eraseWindows.at(pLinAddr);
    const uint evtWmin = evtWindow.Wmin;
    const uint evtWmax = evtWindow.Wmax;
	const std::vector<Address> currentWF({evtWFE, evtWFI});

	const uint j = victimPtr->get_pages_valid();
	const uint k = get_block(evtWFI)->get_pages_empty();
    const uint Wvictim = victimPtr->get_erase_count();
#ifndef NDEBUG
    assert(not block_is_in_vector(victim, currentWF));
    assert(j <= BLOCK_SIZE);
    assert(k <= BLOCK_SIZE);
#endif

	if (j <= k) { // Sufficient space to copy everything to WFI
#ifndef NDEBUG
        //      check_block_address_wdiff(evtWFE);
        //      check_block_address_wdiff(evtWFI);
        //
        //      check_valid_pages();
#endif
        // Normal DWF operation (1A)
        victimPtr->_erase_and_copy(
            event, evtWFI, get_block(evtWFI), [this](const ulong lpn, const Address& newAddr) { map[lpn] = newAddr; },
            [this](const ulong lpn, const uint newPage) { map[lpn].page = newPage; });
        // Count erase
        evtWindow.count_erase(victim);
        // Set next page
        get_next_page(evtWFI);

        // Check erase counter
        if (Wvictim < (evtWmax - 1)) {
            evtWFE = victim;
            // Set next page
            get_next_page(evtWFE);
#ifndef NDEBUG
            //     check_block_address_wdiff(evtWFE);
            //     check_block_address_wdiff(evtWFI);
            //
            //     check_valid_pages();
#endif
        } else {
            // Additional move (1B)
            Address moveBlock = choose_move_block(victim);
            Block* movePtr = get_block(moveBlock);
            uint numMoveValid = movePtr->get_pages_valid();
            movePtr->_erase_and_copy(
                event, victim, victimPtr, [this](const ulong lpn, const Address& newAddr) { map[lpn] = newAddr; },
                [this](const ulong lpn, const uint newPage) { map[lpn].page = newPage; });
            // Count erase
            evtWindow.count_erase(moveBlock);
            // Valid move pages to victim
            // Erase move block
            evtWFE = moveBlock;
            // Set next page
            get_next_page(evtWFE);
            // Increase erase count of move (by 1, to wmin+1)
            controller.stats.erase_block(numMoveValid, movePtr->get_erases_remaining());
            controller.stats.nFTLSwaps = controller.stats.nFTLSwaps + 1;
#ifndef NDEBUG
            // check_block_address_wdiff(evtWFE);
            // check_block_address_wdiff(evtWFI);
            //
            // check_valid_pages();
#endif
        }

	} else {
        // Normal DWF operation (2)
        victimPtr->_erase_and_copy(
            event, evtWFI, get_block(evtWFI), [this](const ulong lpn, const Address& newAddr) { map[lpn] = newAddr; },
            [this](const ulong lpn, const uint newPage) { map[lpn].page = newPage; });
        // Count erase
        evtWindow.count_erase(victim);
        // Modify WFI
		evtWFI = victim;
        // Set next page
        get_next_page(evtWFI);
	}

	controller.stats.erase_block(j, victimPtr->get_erases_remaining());
#ifndef NDEBUG
    //  check_block_address_wdiff(evtWFE);
    //  check_block_address_wdiff(evtWFI);
    //
    //  check_valid_pages();
    //  check_ftl_integrity(event.get_logical_address());
    eraseWindows.at(pLinAddr)._check_window(victim);
#endif
    std::cout << evtWmax << std::endl;
}

void FtlImpl_DWFWDiff::gca_collect(Event& event, Address& victim, const Addresses& doNotPick)
{
	const uint pLinAddr = PlaneAddress{victim}.to_linear_address();
    const auto& eraseWindow = eraseWindows.at(pLinAddr);
    const uint evtWmax = eraseWindow.Wmax;
    Addresses ignoreBlocks;
    Address planeAddress{victim};
    const Addresses eraseW = eraseWindow.get_block_addresses(evtWmax, planeAddress);
    std::copy(eraseW.begin(), eraseW.end(), std::back_inserter(ignoreBlocks));
    std::copy(doNotPick.begin(), doNotPick.end(), std::back_inserter(ignoreBlocks));
    garbage->collect(event, victim, ignoreBlocks, true);
}

Address FtlImpl_DWFWDiff::choose_move_block(const Address& victim)
{
    const PlaneAddress eventPlaneAddr(victim);
    const auto pLinAddr = eventPlaneAddr.to_linear_address();
    const auto& eraseWindow = eraseWindows.at(pLinAddr);
    const uint evtWmin = eraseWindow.Wmin;
    const Addresses& WminAddrs = eraseWindow.get_block_addresses(evtWmin, victim);
	const uint numWmin = WminAddrs.size();
    // Move block is d* blocks at random, that have been erased wmin(t) times
	std::function<bool(const Address&)> ignorePred = [&victim](const Address& candidate) {
		// Same block address, maybe even same page
        return victim.compare(candidate) >= BLOCK;
	};

	uint maxValidPages = std::numeric_limits<uint>::min();
	uint maxValidIdx = 0;
	uint numMaxValidIdx = 0;

    if (dStar == 0) { // Greedy
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

        std::cout << "CHOOSE MOVE FROM [";
        for (const auto& idx : eraseWindow.WIdx) {
            std::cout << " " << idx;
        }
        std::cout << "]" << std::endl;
		uint numTries = 0;
        while (numTries < dStar) {
			const uint idx = RandNrGen::get(numWmin);
			const Address& candidate = WminAddrs.at(idx);
            if (ignorePred(candidate) && numWmin > 1) {
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
