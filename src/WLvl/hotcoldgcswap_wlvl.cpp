/* Copyright 2019 Robin Verschoren */

/* hotcoldgcswap_wlvl.cpp */

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

/* WLvlImpl_HotColdGCSwap class
 *
 * Class implementing a wear leveling scheme which swap 2 random hot and cold blocks with a probability p.
 */

#include "ssd.h"
#include <assert.h>
#include <new>
#include <stdio.h>

using namespace ssd;

WLvlImpl_HotColdGCSwap::WLvlImpl_HotColdGCSwap(FtlParent* ftl, const double p) : Wear_leveler(ftl), p(p) {}

WLvlImpl_HotColdGCSwap::~WLvlImpl_HotColdGCSwap() {}

enum status WLvlImpl_HotColdGCSwap::suggest_WF(Event& evt, Address& /*WFSuggestion*/, Controller& controller,
                                               const std::vector<Address>& doNotPick)
{
    double tmpProb = p;
    while (RandNrGen::get() < tmpProb) // Pick a random block
    {
        Address hot;
        bool selectHot = true;
        Address cold;
        bool selectCold = true;
        do {
            /// TODO Make the plane address explicit? Would that be necessary? Maybe lift this up to
            /// controller level.
            Address candidate(evt.get_address());
            candidate.valid = BLOCK;
            random_block(candidate);
            if (not block_is_in_vector(candidate, doNotPick)) {
                if (FTL->get_block_hotness(candidate)) {
                    if (selectHot) {
                        hot = candidate;
                        selectHot = false;
                    }
                } else if (selectCold) {
                    cold = candidate;
                    selectCold = false;
                }
            }
        } while (selectHot or selectCold);
        // SWAP HOT AND COLD BLOCKS
        FTL->swap(evt, hot, cold);

        const uint intWHot = controller.get_pages_valid(hot);
        const uint intWCold = controller.get_pages_valid(cold);
        controller.stats.swap_blocks(intWHot + intWCold);
        tmpProb = tmpProb - 1.0;
    }
    ///@TODO Better signal for when WF is suggested vs when initiated swap
    return FAILURE;
}
