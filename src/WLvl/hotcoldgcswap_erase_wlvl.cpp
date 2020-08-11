/* Copyright 2020 Robin Verschoren */

/* hotcoldgcswap_erase_wlvl.cpp */

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

/* WLvlImpl_HotColdGCSwap_Erase class
 *
 * Class implementing a wear leveling scheme which swaps 2 random hot and cold blocks with a probability p.
 */

#include "ssd.h"
#include <assert.h>
#include <new>
#include <stdio.h>

using namespace ssd;

WLvlImpl_HotColdGCSwap_Erase::WLvlImpl_HotColdGCSwap_Erase(FtlParent* ftl, const double p) : Wear_leveler(ftl), p(p) {}

WLvlImpl_HotColdGCSwap_Erase::~WLvlImpl_HotColdGCSwap_Erase() {}

enum status WLvlImpl_HotColdGCSwap_Erase::suggest_WF(Event& evt, Address& /*WFSuggestion*/, Controller& controller,
                                                     const std::vector<Address>& doNotPick)
{

    double tmpProb = p;
    while (RandNrGen::get() < tmpProb) // Pick a random block
    {
        std::function<bool(const Address&)> ignoreHot = [&doNotPick, &controller](const Address& addr) {
            return controller.get_block_hotness(addr) or block_is_in_vector(addr, doNotPick);
        };
        std::function<bool(const Address&)> ignoreCold = [&doNotPick, &controller](const Address& addr) {
            return !controller.get_block_hotness(addr) or block_is_in_vector(addr, doNotPick);
        };
        // std::function<ulong(const Address&)> eraseCount = [&controller](const Address& addr) {
        //    return controller.get_block_erase_count(addr);
        //};
        // std::function<ulong(const Address&)> remainingErases = [&controller](const Address& addr) {
        //    return BLOCK_ERASES - controller.get_block_erase_count(addr);
        //};
        Address hot{evt.get_address()};
        // Get highest erase count of hot blocks (least remaining), ignore cold blocks
        // greedy_block_same_plane(hot, remainingErases, ignoreCold);
        greedy_block_same_plane_min_valid_pages_tie_max_erase(controller, hot, ignoreCold);
        Address cold{evt.get_address()};
        // Get lowest erase count of cold blocks, ignore hot blocks
        greedy_block_same_plane_min_valid_pages_tie_min_erase(
            controller, cold, ignoreHot); // greedy_block_same_plane(cold, remainingErases, ignoreHot);
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
