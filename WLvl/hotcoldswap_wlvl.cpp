/* Copyright 2017 Robin Verschoren */

/* ban_wlvl.cpp */

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

/* WLvlImpl_Ban_Prob class
 *
 * Class implementing the Ban wear leveling scheme, which cleans a random block
 * with a probability p.
 */

#include "../ssd.h"
#include <assert.h>
#include <new>
#include <stdio.h>

using namespace ssd;

WLvlImpl_HCSwap::WLvlImpl_Ban_Prob(FtlParent* ftl, const double p)
  : Wear_leveler(ftl)
  , p(p)
{
}

WLvlImpl_Ban_Prob::~WLvlImpl_Ban_Prob()
{
}

enum status
WLvlImpl_Ban_Prob::suggest_WF(Address& WFSuggestion, const std::vector<Block*>& doNotPick)
{
    if (RandNrGen::get() < p) { // Pick a random block
        do {
#ifndef SINGLE_PLANE
            WFSuggestion.package = RandNrGen::get(SSD_SIZE);
            WFSuggestion.die = RandNrGen::get(PACKAGE_SIZE);
#endif
            WFSuggestion.plane = RandNrGen::get(DIE_SIZE);
            WFSuggestion.block = RandNrGen::get(PLANE_SIZE);
        } while (is_WF_victim(FTL->get_block_pointer(WFSuggestion), doNotPick));
#ifdef DEBUG
        std::cout << "INSERTED BAN PROB at" << FTL->controller.stats.numGCErase << std::endl;
#endif
        return SUCCESS;
    }
    return FAILURE;
}
