/* Copyright 2017 Robin Verschoren */

/* dwf_ftl.cpp  */

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

#include <iostream>

using namespace ssd;

void
FtlImpl_SWF::initialize(const ulong numLPN)
{
	// Just assume for now that we have PAGE validity, we'll check it later
	// anyway
	Address addr(0, 0, 0, 0, 0, PAGE);
    WF = addr;
	for (unsigned int lpn = 0; lpn < numLPN; lpn++) {

		bool success = false;
		while (not success) {
#ifndef SINGLE_PLANE
			addr.package = RandNrGen::get(SSD_SIZE);
			addr.die = RandNrGen::get(PACKAGE_SIZE);
			addr.plane = RandNrGen::get(DIE_SIZE);
#endif
			addr.block = RandNrGen::get(PLANE_SIZE);
			assert(addr.check_valid() >= BLOCK);

			Block* block = get_block(addr);

            if (not WF.same_block(addr) and get_next_page(addr) == SUCCESS) {
				Event evt(WRITE, lpn, 1, 0);
				evt.set_address(addr);
				block->write(evt);
                map.push_back(addr);
				success = true;
			}
		}
	}
}

FtlImpl_SWF::FtlImpl_SWF(Controller& controller)
	: FtlParent(controller)
    /*, maxLBA(static_cast<ulong>(std::floor((1.0 - SPARE_FACTOR) * BLOCK_SIZE *
                                           PLANE_SIZE * DIE_SIZE * PACKAGE_SIZE)))*/
    , map()
{
	return;
}

FtlImpl_SWF::~FtlImpl_SWF(void)
{
	return;
}

enum status
FtlImpl_SWF::read(Event& event) {
	controller.stats.numFTLRead++;
	const ulong lpn = event.get_logical_address();
	event.set_address(map[lpn]);

	return SUCCESS;
	// return controller.issue(event);
}

enum status
FtlImpl_SWF::write(Event& event) {
#ifdef DEBUG
    check_ftl_integrity(map.size());
#endif

    const enum status prewritewear =
        wlvl->prewrite(event, controller, { WF });
	controller.stats.numFTLWrite++;
	const ulong lpn = event.get_logical_address();

    /// Invalidate previous page
    get_block(map[lpn])->invalidate_page(map[lpn].page, event.get_start_time() + event.get_time_taken());
    assert(controller.get_page_pointer(map[lpn])->get_state() == INVALID);

    while (get_next_page(WF) != SUCCESS) // Still space in WF
	{
        const std::vector<Address> currentWF = { WF };
        Address victim(map[lpn]);
        if (wlvl->suggest_WF(victim, currentWF) == FAILURE) {
            garbage->collect(event, victim, currentWF);
        }
        Block* victimPtr = get_block(victim);
        const uint j = get_pages_valid(victim);
        assert(j <= BLOCK_SIZE);

        victimPtr->_erase_and_copy(
            event, WF, get_block(WF), // Copy to WF where possible; formal arguments here
        [this, &victim](const ulong lpn, const Address& addr) {
            map[lpn] = addr;
        },
        [this](const ulong lpn, const uint newPage) {
            map[lpn].page = newPage;
        });

		/// Set the new WF
        WF = victim;

        controller.stats.erase_block(j);
        if (controller.stats.get_currentPE() >= victimPtr->get_erases_remaining()) {
			controller.stats.next_currentPE();
		}
    }

	event.set_address(WF); // Tell WF to write to this (next) page
    map[lpn] = WF;

	return SUCCESS;
}

enum status
FtlImpl_SWF::trim(Event& event) {
	controller.stats.numFTLTrim++;
	/// TODO Implement
	const ulong lpn = event.get_logical_address();
	event.set_address(map[lpn]);
    get_block(map[lpn])->invalidate_page(map[lpn].page, event.get_start_time() + event.get_time_taken()); // Should we do this here?
	return SUCCESS;
}


void FtlImpl_SWF::check_ftl_integrity(const ulong numLPN){
    uint numPages = 0;
    //std::cout << "NUMPAGES " << event.get_logical_address() << std::endl;
    for (uint package = 0; package < SSD_SIZE; package++)
    {
        for (uint die = 0; die < PACKAGE_SIZE; die++) {
            for (uint plane = 0; plane < DIE_SIZE; plane++) {
                for (uint b = 0; b < PLANE_SIZE; b++) {
                    //std::cout << "    BLOCK" << b << std::endl;
                    for (uint p = 0; p < BLOCK_SIZE; p++) {
                        Address addr(package, die, plane, b, p, PAGE);
                        if (controller.get_page_pointer(addr)->get_state() ==
                                VALID) {
                            /* std::cout << "\t" << p << " : "
                                       << controller.get_page_pointer(addr)
                                            ->get_logical_address()
                                       << std::endl; */
                            numPages++;
                        }
                    }
                }
            }
        }
    }
    assert(numPages == numLPN);
}
