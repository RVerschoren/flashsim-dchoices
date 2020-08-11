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

#include "ssd.h"
#include <assert.h>
#include <new>
#include <stdio.h>

using namespace ssd;

WLvlImpl_Ban_Prob::WLvlImpl_Ban_Prob(FtlParent* ftl, const double p, const uint d) : Wear_leveler(ftl), p(p), d(d) {}

WLvlImpl_Ban_Prob::~WLvlImpl_Ban_Prob() {}

enum status WLvlImpl_Ban_Prob::suggest_WF(Event& /*evt*/, Address& WFSuggestion, Controller& /*controller*/,
                                          const std::vector<Address>& doNotPick)
{

    while (RandNrGen::get() < p) // Pick a random block
	{
        std::function<bool(const Address&)> ignorePred = [&doNotPick](const Address& addr) {
            return block_is_in_vector(addr, doNotPick);
        };
        std::function<uint(const Address&)> validPages = [this](const Address& addr) {
            return FTL->get_pages_valid(addr);
        };
        d_choices_block_same_plane(d, WFSuggestion, validPages, ignorePred);
        return SUCCESS;
	}
    return FAILURE;
}
