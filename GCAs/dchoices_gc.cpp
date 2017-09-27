/* Copyright 2017 Robin Verschoren */

/* dchoices_gc.cpp  */

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

/* Implements the d-choices GC algorithm (selects a victim block in random fashion). */

#include <assert.h>
#include "../ssd.h"
#include "../util.h"

using namespace ssd;

GCImpl_DChoices::GCImpl_DChoices(FtlParent *ftl, const uint d):
    Garbage_collector(ftl),  d(d), FIFOCounter(0)
{
}

GCImpl_DChoices::~GCImpl_DChoices()
{
}

void GCImpl_DChoices::collect(Address &victimAddress)
{


    if(d == 0) ///@TODO Remove FIFO hack
    {
        FIFOCounter = (FIFOCounter + 1) % PLANE_SIZE;
        victimAddress.block = FIFOCounter;
    }else{
        Address address(victimAddress);
        assert(address.check_valid() >= PLANE);
        address.valid = BLOCK;//Make sure this is the case

        uint minValidPages = BLOCK_SIZE + 1;
        for(uint i = 0; i < d; i++)
        {
            address.package = RandNrGen::getInstance().get(SSD_SIZE);
            address.die = RandNrGen::getInstance().get(PACKAGE_SIZE);
            address.plane = RandNrGen::getInstance().get(DIE_SIZE);
            address.block = RandNrGen::getInstance().get(PLANE_SIZE);
            assert(address.check_valid() >= BLOCK);

            const uint validPages = ftl->get_pages_valid(address);
            if(minValidPages > validPages){
                minValidPages = validPages;
                victimAddress = address;
            }
        }
    }
    victimAddress.valid = PAGE;
}
