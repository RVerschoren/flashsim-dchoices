/* Copyright 2019 Robin Verschoren */

/* hcwf_ftl.cpp  */

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

/* Implements a HCWF FTL with built-in swap */

#include "ssd.h"
#include "util.h"

using namespace ssd;

FtlImpl_HCWFPlusSwap::FtlImpl_HCWFPlusSwap(Controller& controller, HotColdID* hcID, const double swapProbability,
                                           const uint dSwap)
    : FtlImpl_HCSwapWF(controller, hcID, swapProbability, dSwap)
{
}

FtlImpl_HCWFPlusSwap::~FtlImpl_HCWFPlusSwap() = default;

Address FtlImpl_HCWFPlusSwap::choose_swap_block(const Address& victim, bool replacingHWF)
{
    const PlaneAddress pAddr(victim);
    Address& evtHWF = HWF.at(pAddr.to_linear_address());
    Address& evtCWF = CWF.at(pAddr.to_linear_address());
    Address swapBlock(victim);
    const PlaneAddress eventPlaneAddr(victim);
    /// TODO There should be a better way for this: maybe extract a FtlImpl_HCWF::doNotPick-method
    /// returning all blocks which can not be used?
    const std::vector<Address> doNotPick = {evtHWF, evtCWF, victim};
    std::function<bool(const Address&)> ignorePred = [&doNotPick](const Address& candidate) {
        return not block_is_in_vector(candidate, doNotPick);
    };
    std::function<uint(const Address&)> costFunc = [this](const Address& addr) { return get_pages_valid(addr); };
    if (dSwap == 0) {
        greedy_block_same_plane(swapBlock, costFunc, ignorePred);
    } else {
        d_choices_block_same_plane(dSwap, swapBlock, costFunc, ignorePred);
    }
    return swapBlock;
}
