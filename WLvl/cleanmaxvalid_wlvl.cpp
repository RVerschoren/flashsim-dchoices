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

/* WLvlImpl_CleanMaxValid class
 *
 * Class implementing the Ban wear leveling scheme, which cleans a random block
 * with a probability p.
 */

#include "../ssd.h"
#include <assert.h>

using namespace ssd;

WLvlImpl_CleanMaxValid::WLvlImpl_CleanMaxValid(FtlParent* ftl, const double p)
  : Wear_leveler(ftl)
  , p(p)
{
}

WLvlImpl_CleanMaxValid::~WLvlImpl_CleanMaxValid()
{
}

enum status
WLvlImpl_CleanMaxValid::suggest_WF(Address& WFSuggestion, const std::vector<Block*>& doNotPick)
{
    if (RandNrGen::get() < p) { // Pick a random block
        std::function<double(const Address&)> costFunc = [this](const Address& addr) { return 1.0/FTL->get_pages_valid(addr); };
        std::function<bool(const Address&)> ignorePred = [this, &doNotPick](const Address& addr) {
            return block_is_in_vector(FTL->get_block_pointer(addr), doNotPick);
        };
        greedy_block<double>(WFSuggestion, costFunc, ignorePred);

    }
    return FAILURE;
}
