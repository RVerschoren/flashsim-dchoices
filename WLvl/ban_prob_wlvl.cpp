/* Copyright 2017 Robin Verschoren */

/* ban_wlvl.cpp is part of FlashSim. */

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

/* WLvlImpl_Ban class
 * Robin Verschoren 2017
 *
 * Class implementing the Ban wear leveling scheme, which cleans a random block
 * after every tau erasures.
 */

#include "../ssd.h"
#include <assert.h>
#include <new>
#include <stdio.h>

using namespace ssd;

WLvlImpl_Ban::WLvlImpl_Ban(FtlParent* ftl, const ulong tau)
  : Wear_leveler(ftl)
  , tau(tau)
{
}

WLvlImpl_Ban::~WLvlImpl_Ban()
{
}

enum status
WLvlImpl_Ban::insert(const Address& address, Address& WFSuggestion)
{
    if (FTL->controller.stats.numGCErase % tau == 0) { // Pick a random block
#ifndef SINGLE_PLANE
        Address addr(RandNrGen::get(SSD_SIZE), RandNrGen::get(PACKAGE_SIZE),
                     RandNrGen::get(DIE_SIZE), RandNrGen::get(PLANE_SIZE), 0,
                     PAGE);
#else
        Address addr(address.package, address.die, RandNrGen::get(DIE_SIZE),
                     RandNrGen::get(PLANE_SIZE), 0, PAGE);
#endif
        WFSuggestion = addr;
        return SUCCESS;
    }
    return FAILURE;
}
