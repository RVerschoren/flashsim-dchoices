/* Copyright 2017 Robin Verschoren */

/* hcwf_ftl.cpp  */

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
#include <cassert>
#include <cmath>
#include <vector>

using namespace ssd;

PlaneAddress FtlImpl_HCWF::lpnToPlane(ulong lpn)
{
	uint evtPlane = lpn % (DIE_SIZE);
	uint evtDie = (lpn / DIE_SIZE) % PACKAGE_SIZE;
	uint evtPackage = (lpn / (PACKAGE_SIZE * DIE_SIZE)) % SSD_SIZE;
	return {evtPackage, evtDie, evtPlane};
}

void FtlImpl_HCWF::initialize(const ulong numLPN)
{
	/// TODO  Review how to determine first cold block when not assuming 1 plane
	const double f = HOT_FRACTION;
	const uint numBlocks = PLANE_SIZE * DIE_SIZE * PACKAGE_SIZE * SSD_SIZE;
	// maxHotBlocks = static_cast<uint>(std::ceil(f * numBlocks));
	const uint maxHotBlocksPerPlane = static_cast<uint>(std::ceil(f * PLANE_SIZE));
	maxHotBlocks = maxHotBlocksPerPlane * (DIE_SIZE * PACKAGE_SIZE * SSD_SIZE);
	const uint numColdBlocksPerPlane = PLANE_SIZE - maxHotBlocksPerPlane;

	numHotBlocks = 0;
	/// Set hotness
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				PlaneAddress pAddr(package, die, plane);
				HWF[pAddr.to_linear_address()] = Address(package, die, plane, 0, 0, PAGE);
				CWF[pAddr.to_linear_address()] = Address(package, die, plane, maxHotBlocksPerPlane, 0, PAGE);
				for (uint block = 0; block < PLANE_SIZE; block++) {
					const Address blockAddr(package, die, plane, block, 0, BLOCK);
					// set_block_hotness(blockAddr, block < maxHotBlocksPerPlane);
					if (block < maxHotBlocksPerPlane and numHotBlocks < maxHotBlocks) {
						set_block_hotness(blockAddr, true);
						numHotBlocks++;
					} else {
						set_block_hotness(blockAddr, false);
					}
				}
			}
		}
	}
#ifndef NDEBUG
	// Set hotness for all hot WFs
	for (const auto& planeHWF : HWF) {
		assert(get_block_hotness(planeHWF) == true); // set_block_hotness(planeHWF, true);
	}
	// Set hotness for all cold WFs
	for (const auto& planeCWF : CWF) {
		assert(get_block_hotness(planeCWF) == false); // set_block_hotness(planeCWF, false);
	}
#endif

	assert(numHotBlocks <= maxHotBlocks);
	assert(numHotBlocks <= numBlocks);
	// Just assume for now that we have PAGE validity
	Address addr(0, 0, 0, 0, 0, PAGE);
	/// Initialize at random
	for (unsigned int lpn = 0; lpn < numLPN; lpn++) {
		bool success = false;
		const bool lpnIsHot = hcID->is_hot(lpn);
		const PlaneAddress pAddr = lpnToPlane(lpn);
		const ulong pLinAddr = pAddr.to_linear_address();
		addr.plane = pAddr.plane;
		addr.die = pAddr.die;
        addr.package = pAddr.package;
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

FtlImpl_HCWF::FtlImpl_HCWF(Controller& controller, HotColdID* hcID)
	: FtlParent(controller), HWF(SSD_SIZE * PACKAGE_SIZE * DIE_SIZE), CWF(SSD_SIZE * PACKAGE_SIZE * DIE_SIZE),
	  hcID(hcID)
{
}

FtlImpl_HCWF::~FtlImpl_HCWF() = default;

void FtlImpl_HCWF::gca_collect(Event& event, Address& victim, const bool replacingHWF,
							   const std::vector<Address>& doNotPick)
{
    garbage->collect(event, victim, doNotPick, replacingHWF);
}

enum status FtlImpl_HCWF::read(Event& event)
{
	controller.stats.numFTLRead++;
	const ulong lpn = event.get_logical_address();
	event.set_address(map[lpn]);

	return SUCCESS;
}

enum status FtlImpl_HCWF::write(Event& event)
{
#ifndef NDEBUG
	check_valid_pages(map.size());
	check_block_hotness();
    check_ftl_hotness_integrity(event);
#endif

	const ulong lpn = event.get_logical_address();
	PlaneAddress evtPlaneAddr = lpnToPlane(lpn);
	Address& evtHWF = HWF.at(evtPlaneAddr.to_linear_address());
	Address& evtCWF = CWF.at(evtPlaneAddr.to_linear_address());

	wlvl->prewrite(event, controller, {evtHWF, evtCWF}); /// WARNING Returns status, but no action needed on failure

	controller.stats.numFTLWrite++;

	/// Invalidate previous page
	get_block(map[lpn])->invalidate_page(map[lpn].page, event.get_start_time() + event.get_time_taken());

	const bool lpnIsHot = hcID->is_hot(lpn);

	controller.stats.numExternalGCInvocations++;

	int GCChainLength = 0;
	const bool HWFGCChain = get_next_page(evtHWF) != SUCCESS;

	while (get_next_page(evtHWF) != SUCCESS or get_next_page(evtCWF) != SUCCESS) // Still space in WF
	{
		const bool HWFInitiated = get_next_page(evtHWF) != SUCCESS; // HWF initiated cleaning cycle

		// Need to select a victim block through GC
		Address victim = map[lpn];
		const std::vector<Address> currentWF = {evtHWF, evtCWF};

		if (wlvl->suggest_WF(event, victim, controller, currentWF) ==
			FAILURE) { /// BUG Wear leveling still selects from different planes
			this->gca_collect(event, victim, HWFInitiated, currentWF);
		}

		this->merge(event, victim, HWFInitiated, controller.stats);
		GCChainLength++;
	}
	if (lpnIsHot) {
		event.set_address(evtHWF); // Tell WF to write to this (next) page
		map[lpn] = evtHWF;
	} else {
		event.set_address(evtCWF); // Tell WF to write to this (next) page
		map[lpn] = evtCWF;
	}
	if (HWFGCChain) {
		while (GCChainLength >= controller.stats.hotGCChainLength.size() - 1) {
			controller.stats.hotGCChainLength.push_back(0);
		}
		controller.stats.hotGCChainLength[GCChainLength]++;
	} else {
		while (GCChainLength >= controller.stats.coldGCChainLength.size() - 1) {
			controller.stats.coldGCChainLength.push_back(0);
		}
		controller.stats.coldGCChainLength[GCChainLength]++;
	}

#ifndef NDEBUG
	check_valid_pages(map.size() - 1);
	check_block_hotness();
#endif

	return SUCCESS;
}

enum status FtlImpl_HCWF::trim(Event& event)
{
	/// TODO Implement TRIM command
	const ulong lpn = event.get_logical_address();
	event.set_address(map[lpn]);
	// get_block_pointer(map[lpn])->invalidate_page(map[lpn].page); /// WARNING Should we do this here?
	return SUCCESS;
}

void FtlImpl_HCWF::modifyFTL(const ulong lpn, const Address& address) { map[lpn] = address; }

void FtlImpl_HCWF::modifyFTLPage(const ulong lpn, const uint newPage) { map[lpn].page = newPage; }

///@TODO Remove check_* methods, compact into write-method if possible
void FtlImpl_HCWF::check_valid_pages(const ulong numLPN)
{
	uint numPages = 0;
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				for (uint b = 0; b < PLANE_SIZE; b++) {
					// std::cout << "    BLOCK" << b << std::endl;
					for (uint p = 0; p < BLOCK_SIZE; p++) {
						Address addr(package, die, plane, b, p, PAGE);
						if (controller.get_page_pointer(addr)->get_state() == VALID) {
							numPages++;
						}
					}
				}
			}
		}
	}
	assert(numPages == numLPN);
}

void FtlImpl_HCWF::merge(Event& event, const Address& victim, const bool replacingHWF, Stats& stats)
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

	PlaneAddress pAddr(victim);
	Address& evtHWF = HWF.at(pAddr.to_linear_address());
	Address& evtCWF = CWF.at(pAddr.to_linear_address());

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
	stats.erase_block(j, victimPtr->get_erases_remaining(), victimIsHot, victimIsHot ? j : 0, numHotBlocks);
	///@TODO Remove this
	/// if (controller.stats.get_currentPE() >=
	///        victimPtr->get_erases_remaining()) {
	///	controller.stats.next_currentPE();
	///}
}

void FtlImpl_HCWF::check_block_hotness()
{
#ifndef NDEBUG
	uint numBlocks = 0;
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				for (uint b = 0; b < PLANE_SIZE; b++) {
					const Address blockAddr(package, die, plane, b, 0, BLOCK);
					if (get_block_hotness(blockAddr))
						numBlocks++;
				}
			}
		}
	}
	assert(numBlocks == numHotBlocks);
#endif
}

void FtlImpl_HCWF::check_ftl_hotness_integrity(const Event& evt)
{
    for (ulong lpn = 0; lpn < map.size(); lpn++) {
        if (evt.get_event_type() == event_type::WRITE and lpn == evt.get_logical_address())
            continue;
        const Address& addr = map[lpn];
        if (lpn != get_block(addr)->get_page_pointer(addr)->get_logical_address() or
            get_block(addr)->get_page_pointer(addr)->get_state() != VALID or
            get_block_hotness(addr) != hcID->is_hot(lpn)) {
			bool blockHotness = get_block_hotness(addr);
			bool hcHotness = hcID->is_hot(lpn);
			assert(false);
		}
		assert(get_block_hotness(addr) == hcID->is_hot(lpn));
	}
}
