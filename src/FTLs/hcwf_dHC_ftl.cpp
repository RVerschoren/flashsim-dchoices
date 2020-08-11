/* Copyright 2019 Robin Verschoren */

/* hcwf_dHC_ftl.cpp  */

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

/* Implements the HCWF_DHC FTL, based on HCWF */

#include "ssd.h"
#include "util.h"

using namespace ssd;

FtlImpl_HCWF_DHC::FtlImpl_HCWF_DHC(Controller& controller, HotColdID* hcID, const uint dH, const uint dC)
    : FtlImpl_HCWF(controller, hcID), dH(dH), dC(dC)
{
    return;
}

FtlImpl_HCWF_DHC::~FtlImpl_HCWF_DHC(void) { return; }

void FtlImpl_HCWF_DHC::gca_collect(Event& /*event*/, Address& victimAddress, const bool replacingHWF,
                                   const std::vector<Address>& doNotPick)
{
    std::function<uint(const Address&)> validPages = [this](const Address& addr) { return get_pages_valid(addr); };
    std::function<bool(const Address&)> ignorePred = [&doNotPick](const Address& addr) {
        return block_is_in_vector(addr, doNotPick);
    };
    const uint d = (replacingHWF ? dH : dC);
    if (d == 0) {
        greedy_block_same_plane(victimAddress, validPages, ignorePred);
    } else {
        d_choices_block_same_plane(d, victimAddress, validPages, ignorePred);
    }
}
