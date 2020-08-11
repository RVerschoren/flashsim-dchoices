/* Copyright 2019 Robin Verschoren */

/* hcswapwf_ftl.cpp  */

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

/* Implements a HCWF FTL with built-in swap during GCA. */

#include "ssd.h"
#include "util.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <vector>

using namespace ssd;

FtlImpl_HCSwapWF_EraseTie::FtlImpl_HCSwapWF_EraseTie(Controller& controller, HotColdID* hcID, double swap_probability,
                                                     uint d)
    : FtlImpl_HCSwapWF(controller, hcID, swap_probability, d)
{
}

FtlImpl_HCSwapWF_EraseTie::~FtlImpl_HCSwapWF_EraseTie() = default;

Address FtlImpl_HCSwapWF_EraseTie::choose_swap_block(const Address& victim, bool /*replacingHWF*/)
{
	const bool victimHotness = get_block_hotness(victim);
	const PlaneAddress pAddr(victim);
	Address& evtHWF = HWF.at(pAddr.to_linear_address());
	Address& evtCWF = CWF.at(pAddr.to_linear_address());
	Address swapBlock(victim);
    const PlaneAddress eventPlaneAddr(victim);
    /// TODO There should be a better way for this: maybe extract a FtlImpl_HCWF::doNotPick-method
	/// returning all blocks which can not be used?

    const std::vector<Address> doNotPick = {evtHWF, evtCWF, victim};
	std::function<bool(const Address&)> ignorePred = [this, &doNotPick, victimHotness](const Address& candidate) {
		return get_block_hotness(candidate) == victimHotness or block_is_in_vector(candidate, doNotPick);
	};
	if (dSwap == 0) {
        greedy_block_same_plane_min_valid_pages_tie_min_erase(controller, swapBlock, ignorePred);
	} else {
        d_choices_block_same_plane_min_valid_pages_tie_min_erase(controller, dSwap, swapBlock, ignorePred);
	}
	return swapBlock;
}