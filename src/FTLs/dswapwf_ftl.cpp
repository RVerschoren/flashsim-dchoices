/* Copyright 2019 Robin Verschoren */

/* dswapwf_ftl.cpp */

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

/* Implements a DWF  FTL with built-in swap during GCA. */

#include "ssd.h"
#include "util.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace ssd;

#ifndef NDEBUG
void check_block_address2(const Address& blockAddress)
{

	assert(blockAddress.package < SSD_SIZE);
	assert(blockAddress.die < PACKAGE_SIZE);
	assert(blockAddress.plane < DIE_SIZE);
	assert(blockAddress.block < PLANE_SIZE);
	assert(blockAddress.page < BLOCK_SIZE);
	assert(blockAddress.valid <= PAGE);
}
/*
void FtlImpl_DSwapWF::check_ftl_integrity(const ulong lpn)
{
	for (ulong l = 0; l < map.size(); l++) {
		if (l != lpn) {
			Address addr = map[l];
			assert(addr.plane < DIE_SIZE);
			assert(addr.block < PLANE_SIZE);
			assert(addr.page < BLOCK_SIZE);
			assert(controller.get_page_pointer(addr)->get_state() == VALID);
			const ulong la = controller.get_page_pointer(addr)->get_logical_address();
			assert(la == l);
		}
	}
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				PlaneAddress pAddr(package, die, plane);
				assert(controller.get_page_pointer(WFE.at(pAddr.to_linear_address()))->get_state() == EMPTY);
				assert(controller.get_page_pointer(WFI.at(pAddr.to_linear_address()))->get_state() == EMPTY);
				for (uint b = 0; b < PLANE_SIZE; b++) {
					for (uint p = 0; p < BLOCK_SIZE; p++) {
						Address addr(package, die, plane, b, p, PAGE);
						if (controller.get_page_pointer(addr)->get_state() == VALID) {
							const ulong la = controller.get_page_pointer(addr)->get_logical_address();
							assert(la != lpn); // WAS INVALIDATED!
							assert(map[la].block == b);
							assert(map[la].page == p);
						}
					}
				}
			}
		}
	}
}

void FtlImpl_DSwapWF::check_ftl_integrity()
{
	for (ulong l = 0; l < map.size(); l++) {
		Address addr = map[l];
		assert(addr.plane < DIE_SIZE);
		assert(addr.block < PLANE_SIZE);
		assert(addr.page < BLOCK_SIZE);
		if (addr.block < PLANE_SIZE)
			addr.valid = PAGE;
		assert(controller.get_page_pointer(addr)->get_state() == VALID);
		const ulong la = controller.get_page_pointer(addr)->get_logical_address();
		assert(la == l);
	}
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				for (uint b = 0; b < PLANE_SIZE; b++) {
					for (uint p = 0; p < BLOCK_SIZE; p++) {
						Address addr(package, die, plane, b, p, PAGE);
						if (controller.get_page_pointer(addr)->get_state() == VALID) {
							const ulong la = controller.get_page_pointer(addr)->get_logical_address();
							assert(map[la].block == b);
							assert(map[la].page == p);
						}
					}
				}
			}
		}
	}
}
*/
// void FtlImpl_DSwapWF::check_valid_pages(const ulong /*numLPN*/)
/*{
	uint numPages = 0;
	// std::cout << "NUMPAGES " << event.get_logical_address() << std::endl;
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				for (uint b = 0; b < PLANE_SIZE; b++) {
					// std::cout << "    BLOCK" << b << std::endl;
					for (uint p = 0; p < BLOCK_SIZE; p++) {
						Address addr(package, die, plane, b, p, PAGE);
						if (controller.get_page_pointer(addr)->get_state() == VALID) {
							// std::cout << "\t" << p << " : " <<
							// controller.get_page_pointer(addr)->get_logical_address()
							// << std::endl;
							numPages++;
						}
					}
				}
			}
		}
	}
	/// TODO assert(numPages == numLPN);
}

void FtlImpl_DSwapWF::check_hot_pages(Address blockAddr, Block* blockPtr, const uint hotPages)
{
	uint hotVictimPages = 0;
	std::cout << "Check hot pages block " << blockAddr.block << std::endl;
	for (uint p = 0; p < BLOCK_SIZE; p++) {
		blockAddr.page = p;
		const Page* page = blockPtr->get_page_pointer(blockAddr);
		const bool pageIsValid = page->get_state() == VALID;
		const bool pageIsHot = hcID->is_hot(page->get_logical_address());
		if (pageIsValid) {
			std::cout << page->get_logical_address() << " is hot: " << pageIsHot << std::endl;
			if (pageIsHot) {
				hotVictimPages++;
			}
		}
	}
	std::cout << "End check" << blockAddr.block << std::endl;
	assert(hotVictimPages == hotPages);
	}*/
#endif

/// TODO Merge this with FtlImpl_HCWF equivalent
/// This is stupid
PlaneAddress lpnToPlane2(ulong lpn)
{
	uint evtPlane = lpn % (DIE_SIZE);
	uint evtDie = (lpn / DIE_SIZE) % PACKAGE_SIZE;
	uint evtPackage = (lpn / (PACKAGE_SIZE * DIE_SIZE)) % SSD_SIZE;
	return {evtPackage, evtDie, evtPlane};
}

void FtlImpl_DSwapWF::initialize(const ulong numLPN)
{
	// Just assume for now that we have PAGE validity, we'll check it later
	// anyway
	const double f = HOT_FRACTION;
	const uint maxHotBlocksPerPlane = static_cast<uint>(std::ceil(f * PLANE_SIZE));
	numHotBlocks = 0;
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				const PlaneAddress pAddr(package, die, plane);
				const ulong pLinAddr = pAddr.to_linear_address();
				WFE[pLinAddr] = Address(package, die, plane, 0, 0, PAGE);
				get_block(WFE[pLinAddr])->set_block_hotness(true);
				// numHotBlocks++;
				WFI[pLinAddr] = Address(package, die, plane, 1, 0, PAGE);
				get_block(WFI[pLinAddr])->set_block_hotness(false);
				uint numHotBlocksPerPlane = 1;

				Address blockAddr(package, die, plane, 2, 0, PAGE);
				for (uint block = 2; block < PLANE_SIZE; block++) {
					blockAddr.block = block;
					if (numHotBlocksPerPlane < maxHotBlocksPerPlane) {
						get_block(blockAddr)->set_block_hotness(true);
						numHotBlocksPerPlane++;
					} else {
						get_block(blockAddr)->set_block_hotness(false);
					}
				}
				numHotBlocks += numHotBlocksPerPlane;
			}
		}
	}
	std::cout << "NUMBER HOT BLOCKS " << numHotBlocks << " out of " << maxHotBlocksPerPlane << std::endl;

	Address addr(0, 0, 0, 0, 0, PAGE);
	for (ulong lpn = 0; lpn < numLPN; lpn++) {
		bool success = false;
		// const bool lpnIsHot = hcID.is_hot(lpn);
		const PlaneAddress pAddr = lpnToPlane2(lpn);
		const ulong pLinAddr = pAddr.to_linear_address();
		addr.package = pAddr.package;
		addr.die = pAddr.die;
		addr.plane = pAddr.plane;
		while (not success) {
			random_block_same_plane(addr);
			assert(addr.check_valid() >= BLOCK);
			assert(addr.block < PLANE_SIZE);

			Block* block = get_block(addr);

			if (not WFE[pLinAddr].same_block(addr) and not WFI[pLinAddr].same_block(addr) and
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

FtlImpl_DSwapWF::FtlImpl_DSwapWF(Controller& controller, HotColdID* hcID, const double swapProbability,
								 const uint dSwap)
	: FtlImpl_DWF(controller, hcID), dSwap(dSwap), pSwap(swapProbability), numHotBlocks(0)
///@TODO Enable hotvalidpages for DWF
/// , hotValidPages(SSD_SIZE, std::vector<std::vector<std::vector<uint>>>(
///            PACKAGE_SIZE, std::vector<std::vector<uint>>(DIE_SIZE,
///            std::vector<uint>(PLANE_SIZE,0UL))))
{
}

FtlImpl_DSwapWF::~FtlImpl_DSwapWF() = default;

enum status FtlImpl_DSwapWF::_erase_swap_and_copy(Event& event, Address& victimBlock, Block* victimPtr,
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
    check_valid_pages();
	// check_block_hotness();
    // check_ftl_hotness_integrity();
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
    // check_ftl_hotness_integrity();
#endif
	std::vector<ulong> swapLpns = swapPtr->_read_logical_addresses_and_data(event);
	swapPtr->_erase(event);
	const bool swapBlockHotness = not victimBlockHotness;
	const uint swapValidPages = static_cast<unsigned int>(swapLpns.size());
	stats.erase_block(swapValidPages, swapPtr->get_erases_remaining(), swapBlockHotness,
					  swapBlockHotness ? swapValidPages : 0, numHotBlocks);

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

    check_valid_pages();
    //	check_block_hotness();
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
}

void FtlImpl_DSwapWF::merge(Event& event, const Address& victim)
{

	Block* victimPtr = get_block(victim);
	PlaneAddress pAddr(victim);
	Address& evtWFE = WFE.at(pAddr.to_linear_address());
	Address& evtWFI = WFI.at(pAddr.to_linear_address());
	const std::vector<Address> currentWF({evtWFE, evtWFI});

	const uint j = victimPtr->get_pages_valid();
	const uint k = get_block(evtWFI)->get_pages_empty();

#ifndef NDEBUG
	assert(not block_is_in_vector(victim, currentWF));
	assert(j <= BLOCK_SIZE);
	assert(k <= BLOCK_SIZE);
	check_block_address2(evtWFE);
	check_block_address2(evtWFI);
	check_block_address2(victim);
#endif
	// Block is last used as WFE?
	const bool victimWasWFE = get_block_hotness(victim);
	if (j <= k) { // Sufficient space to copy everything to WFI
                  /// TODO Enable hotValidPages
        victimPtr->_erase_and_copy(
            event, evtWFI, get_block(evtWFI), [this](const ulong lpn, const Address& newAddr) { map[lpn] = newAddr; },
            [this](const ulong lpn, const uint newPage) { map[lpn].page = newPage; });
		evtWFE = victim;
		if (!victimWasWFE) {
			numHotBlocks++;
		}
	} else {
		// Ex WFE replaces WFI, swap
		/// TODO Enable hotValidPages
		/// const uint hotPages =
		/// hotValidPages[WFI.package][WFI.die][WFI.plane][WFI.block];
		/// controller.stats.WFIHotPagesDist[hotPages] =
		/// controller.stats.WFIHotPagesDist[hotPages] + 1;
		/// Choose swap block
		Address victimBlock(victim); // Bypass const
		Address swapBlock = choose_swap_block(victim);
		const bool swapWasWFE = get_block(swapBlock)->get_block_hotness();
		/// Initiate swap if possible
		if (RandNrGen::get() < pSwap and get_block_hotness(swapBlock) != victimWasWFE) {
            _erase_swap_and_copy(
                event, victimBlock, victimPtr, evtWFI, get_block(evtWFE), swapBlock,
                [this](const ulong lpn, const Address& newAddr) { map[lpn] = newAddr; }, victimWasWFE,
                controller.stats);
			if (victimWasWFE) {
				evtWFE = swapBlock;
				set_block_hotness(evtWFE, true);
				if (!swapWasWFE) {
					numHotBlocks++;
				}
				// Replace sameWF with (swapped) victim
				evtWFI = victimBlock;
				set_block_hotness(evtWFI, false);
				if (victimWasWFE) {
					numHotBlocks--;
				}
			} else {
				evtWFI = swapBlock;
				set_block_hotness(evtWFI, false);
				if (swapWasWFE) {
					numHotBlocks--;
				}
				// Replace sameWF with (swapped) victim
				evtWFE = victimBlock;
				set_block_hotness(evtWFE, true);
				if (victimWasWFE) {
					numHotBlocks++;
				}
			}
		} else {
            victimPtr->_erase_and_copy(
                event, evtWFI, get_block(evtWFI),
                [this](const ulong lpn, const Address& newAddr) { map[lpn] = newAddr; },
                [this](const ulong lpn, const uint newPage) { map[lpn].page = newPage; });
			if (victimWasWFE) {
				numHotBlocks--;
			}
			evtWFI = victim;
		}
	}
	//}
	// else
	//{ // Victim was WFI

	//	/// Choose swap block
	//	Address victimBlock(victim); // Bypass const
	//	Address swapBlock = choose_swap_block(victim);
	//	/// Initiate swap if possible
	//	if (RandNrGen::get() < pSwap and get_block_hotness(swapBlock) != victimIsHot) {
	//		_erase_swap_and_copy(event, victimBlock, victimPtr, otherWF, get_block(otherWF), swapBlock, modifyFTL,
	//							 replacingHWF, stats);
	//		// Previous otherHWF is full (because of copies from victim)
	//		otherWF = swapBlock;
	//		set_block_hotness(otherWF, !replacingHWF);
	//		// Replace sameWF with (swapped) victim
	//		sameWF = victimBlock;
	//		set_block_hotness(sameWF, replacingHWF);
	//	} else {
	//		victimPtr->_erase_and_copy(event, otherWF,
	//								   get_block(otherWF), // Copy to other WF where possible
	//								   modifyFTL, modifyFTLPage);
	//		otherWF = victim;
	//		set_block_hotness(otherWF, !replacingHWF);
	//	}

	//}
	// std::cout << "AFTER MERGE STATS:" << victimPtr->get_erases_remaining() << std::endl;
	controller.stats.erase_block(j, victimPtr->get_erases_remaining());
	//	controller.stats.erase_block(j, victimPtr->get_erases_remaining(), victimIsHot, victimIsHot ? j : 0,
	// numHotBlocks);
}

Address FtlImpl_DSwapWF::choose_swap_block(const Address& victim)
{
	const bool victimHotness = get_block_hotness(victim);
	const PlaneAddress pAddr(victim);
	Address& evtWFE = WFE.at(pAddr.to_linear_address());
	Address& evtWFI = WFI.at(pAddr.to_linear_address());
	Address swapBlock(victim);
	const PlaneAddress eventPlaneAddr(victim);
	/// TODO There should be a better way for this: maybe extract a FtlImpl_HCWF::doNotPick-method
	/// returning all blocks which can not be used?
	const std::vector<Address> doNotPick = {evtWFE, evtWFI, victim};
	std::function<bool(const Address&)> ignorePred = [this, &doNotPick, victimHotness](const Address& candidate) {
		/// WARNING DO we have to enable this again?
		return get_block_hotness(candidate) == victimHotness and not block_is_in_vector(candidate, doNotPick);
	};
	std::function<uint(const Address&)> costFunc = [this](const Address& addr) { return get_pages_valid(addr); };
	if (dSwap == 0) {
		greedy_block_same_plane(swapBlock, costFunc, ignorePred);
	} else {
		d_choices_block_same_plane(dSwap, swapBlock, costFunc, ignorePred);
	}

	return swapBlock;
}
