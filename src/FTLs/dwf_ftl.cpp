/* Copyright 2017 Robin Verschoren */

/* dwf_ftl.cpp */

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
#include <assert.h>
#include <math.h>
#include <new>
#include <stdio.h>

using namespace ssd;

#ifdef DEBUG
void
check_block_address(const Address& blockAddress)
{

	assert(blockAddress.package < SSD_SIZE);
	assert(blockAddress.die < PACKAGE_SIZE);
	assert(blockAddress.plane < DIE_SIZE);
	assert(blockAddress.block < PLANE_SIZE);
	assert(blockAddress.page < BLOCK_SIZE);
	assert(blockAddress.valid <= PAGE);
}

void
FtlImpl_DWF::check_ftl_integrity(const ulong lpn)
{
	for (ulong l = 0; l < map.size(); l++) {
		if (l != lpn) {
			Address addr = map[l];
			assert(addr.plane < DIE_SIZE);
			assert(addr.block < PLANE_SIZE);
			assert(addr.page < BLOCK_SIZE);
			assert(controller.get_page_pointer(addr)->get_state() == VALID);
			const ulong la =
			    controller.get_page_pointer(addr)->get_logical_address();
			assert(la == l);
		}
	}
	assert(controller.get_page_pointer(WFE)->get_state() == EMPTY);
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				for (uint b = 0; b < PLANE_SIZE; b++) {
					for (uint p = 0; p < BLOCK_SIZE; p++) {
						Address addr(package, die, plane, b, p, PAGE);
						if (controller.get_page_pointer(addr)->get_state() ==
						        VALID) {
							const ulong la = controller.get_page_pointer(addr)
							                 ->get_logical_address();
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

void
FtlImpl_DWF::check_ftl_integrity()
{
	for (ulong l = 0; l < map.size(); l++) {
		Address addr = map[l];
		assert(addr.plane < DIE_SIZE);
		assert(addr.block < PLANE_SIZE);
		assert(addr.page < BLOCK_SIZE);
		if (addr.block < PLANE_SIZE)
			addr.valid = PAGE;
		assert(controller.get_page_pointer(addr)->get_state() == VALID);
		const ulong la =
		    controller.get_page_pointer(addr)->get_logical_address();
		assert(la == l);
	}
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				for (uint b = 0; b < PLANE_SIZE; b++) {
					for (uint p = 0; p < BLOCK_SIZE; p++) {
						Address addr(package, die, plane, b, p, PAGE);
						if (controller.get_page_pointer(addr)->get_state() ==
						        VALID) {
							const ulong la = controller.get_page_pointer(addr)
							                 ->get_logical_address();
							assert(map[la].block == b);
							assert(map[la].page == p);
						}
					}
				}
			}
		}
	}
}

void
FtlImpl_DWF::check_valid_pages(const ulong /*numLPN*/)
{
	uint numPages = 0;
	// std::cout << "NUMPAGES " << event.get_logical_address() << std::endl;
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				for (uint b = 0; b < PLANE_SIZE; b++) {
					// std::cout << "    BLOCK" << b << std::endl;
					for (uint p = 0; p < BLOCK_SIZE; p++) {
						Address addr(package, die, plane, b, p, PAGE);
						if (controller.get_page_pointer(addr)->get_state() ==
						        VALID) {
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

void
FtlImpl_DWF::check_hot_pages(Address blockAddr, Block* blockPtr, const uint hotPages)
{
	uint hotVictimPages = 0;
	std::cout << "Check hot pages block " << blockAddr.block << std::endl;
	for (uint p = 0; p < BLOCK_SIZE; p++) {
		blockAddr.page = p;
		const Page* page = blockPtr->get_page_pointer(blockAddr);
		const bool pageIsValid = page->get_state() == VALID;
		const bool pageIsHot = hcID->is_hot(page->get_logical_address());
		if (pageIsValid) {
			std::cout << page->get_logical_address() << " is hot: " << pageIsHot
			          << std::endl;
			if (pageIsHot) {
				hotVictimPages++;
			}
		}
	}
	std::cout << "End check" << blockAddr.block << std::endl;
	assert(hotVictimPages == hotPages);
}
#endif

void
FtlImpl_DWF::initialize(const ulong numLPN)
{
	// Just assume for now that we have PAGE validity, we'll check it later
	// anyway
	Address addr(0, 0, 0, 0, 0, PAGE);
	WFE = addr;
	WFI = Address(0, 0, 0, 1, 0, PAGE);

	for (ulong lpn = 0; lpn < numLPN; lpn++) {
		bool success = false;
		// const bool lpnIsHot = hcID.is_hot(lpn);
		while (not success) {
			random_block(addr);
			assert(addr.check_valid() >= BLOCK);
			assert(addr.block < PLANE_SIZE);

			Block* block = get_block(addr);

			if (not WFE.same_block(addr) and not WFI.same_block(addr) and
			        block->get_next_page(addr) == SUCCESS) {
				Event evt(WRITE, lpn, 1, 0);
				evt.set_address(addr);
				block->write(evt);
				map.push_back(addr);
				/// TODO if (hcID->is_hot(lpn)) {
				/// TODO
				/// hotValidPages[addr.package][addr.die][addr.plane][addr.block]++;
				/// TODO }
				success = true;
			}
		}
	}

#ifdef DEBUG
	check_valid_pages(numLPN);
	check_ftl_integrity();
	Address a(0, 0, 0, 0, 0, PAGE);
	for (uint i = 0; i < PLANE_SIZE; i++) {
		a.block = i;
		/// TODO check_hot_pages(a, get_block_pointer(a),
		/// hotValidPages[a.package][a.die][a.plane][a.block]);
	}
#endif
}

FtlImpl_DWF::FtlImpl_DWF(Controller& controller, HotColdID* hcID)
	: FtlParent(controller)
	, map()
	, hcID(hcID)
/// TODO , hotValidPages(SSD_SIZE, std::vector<std::vector<std::vector<uint>>>(
/// TODO                             PACKAGE_SIZE,
/// std::vector<std::vector<uint>>(DIE_SIZE, std::vector<uint>(PLANE_SIZE,
/// 0UL))))
{
	return;
}

FtlImpl_DWF::~FtlImpl_DWF(void)
{
	return;
}

enum status
FtlImpl_DWF::read(Event& event) {
	controller.stats.numFTLRead++;
	const ulong lpn = event.get_logical_address();
	event.set_address(map[lpn]);
	return SUCCESS;
}

void FtlImpl_DWF::recompute_hotvalidpages(Address /*block*/)
{
	/*Block* blockPtr = get_block_pointer(block);
	uint hotPages = 0;
	for (uint p = 0; p < BLOCK_SIZE; p++) {
	    block.page = p;
	    const Page* page = blockPtr->get_page_pointer(block);
	    if (page->get_state() == VALID and
	hcID->is_hot(page->get_logical_address())) {
	        hotPages++;
	    }
	}
	///TODO hotValidPages[block.package][block.die][block.plane][block.block] =
	hotPages;
	assert(hotPages <= BLOCK_SIZE);
    #ifdef DEBUG
	check_hot_pages(block, blockPtr, hotPages);
	#endif*/
}

void
FtlImpl_DWF::modifyFTL(const ulong lpn, const Address& newAddr)
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
    /// TODO remove//oldAddr.valid = PAGE;
	const bool pageWasValid = get_state(oldAddr) == VALID;
	const bool pageIsHot = false; /// TODO hcID->is_hot(lpn);
	if (pageWasValid and pageIsHot) {
        // hotValidPages[oldAddr.package][oldAddr.die][oldAddr.plane][oldAddr.block] -= 1;
        // hotValidPages[newAddr.package][newAddr.die][newAddr.plane][newAddr.block] += 1;
		recompute_hotvalidpages(oldAddr);
		recompute_hotvalidpages(newAddr);
	}
	assert(map.at(lpn).package < SSD_SIZE);
	assert(map.at(lpn).die < PACKAGE_SIZE);
	assert(map.at(lpn).block < PLANE_SIZE);
	assert(map.at(lpn).plane < DIE_SIZE);
	assert(map.at(lpn).page < BLOCK_SIZE);
	assert(map.at(lpn).valid <= PAGE);
    //    assert(hotValidPages[oldAddr.package][oldAddr.die][oldAddr.plane][oldAddr.block] >= 0);
    //    assert(hotValidPages[oldAddr.package][oldAddr.die][oldAddr.plane][oldAddr.block] <= BLOCK_SIZE);
    //    assert(hotValidPages[newAddr.package][newAddr.die][newAddr.plane][newAddr.block] >= 0);
    //    assert(hotValidPages[newAddr.package][newAddr.die][newAddr.plane][newAddr.block] <= BLOCK_SIZE);
}

void
FtlImpl_DWF::modify_ftl_page(const ulong lpn, const uint newPage)
{
	assert(map.at(lpn).package < SSD_SIZE);
	assert(map.at(lpn).die < PACKAGE_SIZE);
	assert(map.at(lpn).block < PLANE_SIZE);
	assert(map.at(lpn).plane < DIE_SIZE);
	assert(map.at(lpn).page < BLOCK_SIZE);
	map[lpn].page = newPage;
	assert(map.at(lpn).package < SSD_SIZE);
	assert(map.at(lpn).die < PACKAGE_SIZE);
	assert(map.at(lpn).block < PLANE_SIZE);
	assert(map.at(lpn).plane < DIE_SIZE);
	assert(map.at(lpn).page < BLOCK_SIZE);
	assert(map.at(lpn).valid <= PAGE);
}

enum status
FtlImpl_DWF::write(Event& event) {
	// const enum status prewritewear =
	wlvl->prewrite(event, controller, { WFE, WFI });

	controller.stats.numFTLWrite++;
	const ulong lpn = event.get_logical_address();

	/// Invalidate previous page
	const Address prevAddress(map[lpn]);
	const bool pageWasValid = get_state(prevAddress) == VALID;

	if (pageWasValid)
        get_block(prevAddress)->invalidate_page(prevAddress.page, event.get_start_time() + event.get_time_taken());
/// TODO if (pageWasValid and hcID->is_hot(lpn)) {
/// TODO hotpages
///}
#ifdef DEBUG
	check_block_address(prevAddress);
	assert(get_state(prevAddress) != VALID); // Invalidated
	check_block_address(WFE);
	check_block_address(WFI);
#endif
	while (get_next_page(WFE) != SUCCESS) // No space in WFE
	{
		const std::vector<Address> currentWF({ WFE, WFI });

		/// Need to select a victim block through GC
		// Temporary address until we find a suitable real victim
		Address victim(map[lpn]);
		assert(victim.check_valid() >= BLOCK);

		if (wlvl->suggest_WF(victim, currentWF) == FAILURE) {
			garbage->collect(event, victim, currentWF);
		}
		Block* victimPtr = get_block(victim);

		const uint j = victimPtr->get_pages_valid();
		const uint k = get_block(WFI)->get_pages_empty();

#ifdef DEBUG
		assert(not block_is_in_vector(victim, currentWF));
		assert(j <= BLOCK_SIZE);
		assert(k <= BLOCK_SIZE);
		check_block_address(WFE);
		check_block_address(WFI);
		check_block_address(victim);
#endif
		victimPtr->_erase_and_copy(
		    event, WFI, get_block(WFI),
		[this](const ulong lpn, const Address& newAddr) {
			map[lpn] = newAddr;
		},
		[this](const ulong lpn, const uint newPage) {
			map[lpn].page = newPage;
		});

		/// TODO recompute_hotvalidpages(victim);

		if (j <= k) { // Sufficient space to copy everything to WFI
			/// TODO hotpages
			WFE = victim;
		} else {
			/// TODO const uint hotPages =
			/// hotValidPages[WFI.package][WFI.die][WFI.plane][WFI.block];
			/// TODO controller.stats.WFIHotPagesDist[hotPages] =
			/// controller.stats.WFIHotPagesDist[hotPages] + 1;
			WFI = victim;
		}

		controller.stats.erase_block(j);
		if (controller.stats.get_currentPE() >=
		        victimPtr->get_erases_remaining()) {
			controller.stats.next_currentPE();
		}
	}

#ifdef DEBUG
	check_block_address(WFE);
	check_block_address(WFI);
#endif
	event.set_address(WFE); // Tell WF to write to this (next) page
	map[lpn] = WFE;
	/// TODO if (hcID->is_hot(lpn)) {
	/// TODO      hotvalidpages
	/// TODO }
	assert(controller.get_page_pointer(WFE)->get_state() == EMPTY);

	return SUCCESS;
}

enum status
FtlImpl_DWF::trim(Event& event) {
	controller.stats.numFTLTrim++;
	/// TODO Implement
	const ulong lpn = event.get_logical_address();
	event.set_address(map[lpn]);
	// get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);//Should
	// we
	// do this here?
	return SUCCESS;
}
