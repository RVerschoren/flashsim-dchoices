/* Copyright 2017 Robin Verschoren */

/* cold_ftl.cpp  */

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

/* Implements the COLD FTL, based on HCWF */

#include "ssd.h"
#include "util.h"

using namespace ssd;

FtlImpl_COLD::FtlImpl_COLD(Controller& controller, HotColdID* hcID, const uint d) : FtlImpl_HCWF(controller, hcID), d(d)
{
}

FtlImpl_COLD::~FtlImpl_COLD() = default;

void FtlImpl_COLD::gca_collect(Event& event, Address& victimAddress, const bool replacingHWF,
							   const std::vector<Address>& doNotPick)
{
    if (replacingHWF) {
		FtlImpl_HCWF::gca_collect(event, victimAddress, replacingHWF, doNotPick);
	} else { // If replacing CWF, pick a cold block (limits hot->cold conversions)
        // Lower priority for hot blocks
        // std::function<uint(const Address&)> costFunc = [this](const Address& addr) {
        //    return get_pages_valid(addr) + (get_block_hotness(addr) ? (BLOCK_SIZE + 1) : 0);
        //};
        std::function<uint(const Address&)> costFunc = [this](const Address& addr) {
            return get_pages_valid(addr) + (get_block_hotness(addr) ? (BLOCK_SIZE + 1) : 0);
		};
        std::function<bool(const Address&)> ignorePred = [&doNotPick](const Address& addr) {
            return block_is_in_vector(addr, doNotPick);
        };
        d_choices_block_same_plane(d, victimAddress, costFunc, ignorePred);
	}
}
