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

GCImpl_FIFO::GCImpl_FIFO(FtlParent *ftl, const Address startAddress):
    Garbage_collector(ftl),  currentAddress(startAddress)
{
}

GCImpl_FIFO::~GCImpl_FIFO()
{
}

void GCImpl_FIFO::collect(Address &victimAddress)
{
        currentAddress.block = (currentAddress.block + 1) % PLANE_SIZE;
        //Cascading, flowing into next plane, die or package if necessary
        if(currentAddress.block == 0)
        {
            currentAddress.plane = (currentAddress.plane + 1) % DIE_SIZE;
            if(currentAddress.plane == 0)
            {
                currentAddress.die = (currentAddress.die + 1) % PACKAGE_SIZE;
                if(currentAddress.die== 0)
                {
                    currentAddress.package = (currentAddress.package + 1) % SSD_SIZE;
                }
            }
        }
        victimAddress = currentAddress;
}
