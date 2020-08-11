/* Copyright 2017 Robin Verschoren */

/* hotcoldswap_wlvl.cpp */

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

/* WLvlImpl_HotColdSwap class
 *
 * Class implementing a wear leveling scheme which swaps the contents of 2 random blocks with a probability p.
 */

#include "ssd.h"
#include <assert.h>
#include <new>
#include <stdio.h>

using namespace ssd;

WLvlImpl_HotColdSwap::WLvlImpl_HotColdSwap(FtlParent* ftl, const double p) : Wear_leveler(ftl), p(p) {}

WLvlImpl_HotColdSwap::~WLvlImpl_HotColdSwap() {}

enum status WLvlImpl_HotColdSwap::suggest_WF(Event& /*evt*/, Address& /*WFSuggestion*/, Controller& /*controller*/,
                                             const std::vector<Address>& /*doNotPick*/)
{
	return FAILURE;
}

enum status WLvlImpl_HotColdSwap::prewrite(Event& evt, Controller& controller, const std::vector<Address>& doNotPick)
{
    auto finishedWLvl = FAILURE;
    double tmpProb = p;
    while (RandNrGen::get() < tmpProb) // Pick a random block
	{
		Address hot;
		bool selectHot = true;
		Address cold;
		bool selectCold = true;
		do {

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
#ifndef NDEBUG
// std::cout << "INSERTED HOTCOLD SWAP at" << FTL->controller.stats.numGCErase
// << std::endl;
#endif
        tmpProb = tmpProb - 1.0;
        finishedWLvl = SUCCESS;
	}
    return finishedWLvl;
}
