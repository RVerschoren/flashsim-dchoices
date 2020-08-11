/* Copyright 2017 Robin Verschoren */

/* randomswap_wlvl.cpp */

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

/* WLvlImpl_RandomSwap class
 *
 * Class implementing the swapping wear leveling scheme, which swaps the
 * contents of 2 randomly selected blocks
 * with a probability p.
 */

#include "ssd.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

using namespace ssd;

WLvlImpl_RandomSwap::WLvlImpl_RandomSwap(FtlParent* ftl, const double p) : Wear_leveler(ftl), p(p) {}

WLvlImpl_RandomSwap::~WLvlImpl_RandomSwap() = default;

enum status WLvlImpl_RandomSwap::suggest_WF(Event& /*evt*/, Address& /*WFSuggestion*/, Controller& /*controller*/,
											const std::vector<Address>& /*doNotPick*/)
{
	return FAILURE;
}

enum status WLvlImpl_RandomSwap::prewrite(Event& evt, Controller& controller, const std::vector<Address>& doNotPick)
{
    auto finishedWLvl = FAILURE;
    double tmpProb = p;
    while (RandNrGen::get() < tmpProb) {

		// Pick a random block
		Address block1(0, 0, 0, 0, 0, BLOCK), block2(0, 0, 0, 0, 0, BLOCK);
		random_block(block1, doNotPick);
		std::vector<Address> doNotPick2(doNotPick);
		doNotPick2.push_back(block1);
		random_block(block2, doNotPick2);

		assert(block1.package < SSD_SIZE);
		assert(block1.die < PACKAGE_SIZE);
		assert(block1.plane < DIE_SIZE);
		assert(block1.block < PLANE_SIZE);
		assert(block2.package < SSD_SIZE);
		assert(block2.die < PACKAGE_SIZE);
		assert(block2.plane < DIE_SIZE);
		assert(block2.block < PLANE_SIZE);

		FTL->swap(evt, block1, block2);

		const uint intW1 = controller.get_pages_valid(block1);
		const uint intW2 = controller.get_pages_valid(block2);
		controller.stats.swap_blocks(intW1 + intW2);
#ifndef NDEBUG
		std::cout << "INSERTED RANDOM SWAP at" << controller.stats.numGCErase << std::endl;
#endif
        tmpProb = tmpProb - 1.0;
        finishedWLvl = SUCCESS;
	}
    return finishedWLvl;
}
