/* Copyright 2017 Robin Verschoren */

/* greedy_gc.cpp  */

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

/* Implements the greedy GC algorithm. */

#include "../ssd.h"
#include "../util.h"
#include <assert.h>

using namespace ssd;

GCImpl_Greedy::GCImpl_Greedy(FtlParent* ftl, const uint d)
  : Garbage_collector(ftl)
  , d(d)
  , FIFOCounter(0)
{
}

GCImpl_Greedy::~GCImpl_Greedy()
{
}

void
GCImpl_Greedy::collect(Address& victimAddress,
                         const std::vector<Block*>& doNotPick)
{
        do {
            Address address(victimAddress);
            assert(address.check_valid() >= PLANE);
            address.valid = BLOCK; // Make sure this is the case

            uint minValidPages = BLOCK_SIZE + 1;
            for (uint i = 0; i < d; i++) {
#ifndef SINGLE_PLANE
                address.package = RandNrGen::get(SSD_SIZE);
                address.die = RandNrGen::get(PACKAGE_SIZE);
#endif
                address.plane = RandNrGen::get(DIE_SIZE);
                address.block = RandNrGen::get(PLANE_SIZE);
                assert(address.check_valid() >= BLOCK);

                const uint validPages = ftl->get_pages_valid(address);
                if (minValidPages > validPages) {
                    minValidPages = validPages;
                    victimAddress = address;
                }
            }
        } while (
          is_WF_victim(ftl->get_block_pointer(victimAddress), doNotPick));

    victimAddress.valid = PAGE;
}
