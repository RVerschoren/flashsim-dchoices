/* Copyright 2018 Robin Verschoren */

/* hcwffalse_ftl.cpp  */

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

/* Implements a HCWF mechanism with possibly (random) false hot/cold identification*/

#include "ssd.h"
#include "util.h"

using namespace ssd;

FtlImpl_HCWFFalse::FtlImpl_HCWFFalse(Controller& controller, HotColdID* hcID, double false_pos_prob,
                                     double false_neg_prob)
    : FtlImpl_HCWF(controller, hcID), prob_fpos(false_pos_prob), prob_fneg(false_neg_prob)
{
    return;
}

FtlImpl_HCWFFalse::~FtlImpl_HCWFFalse(void) { return; }

enum status FtlImpl_HCWFFalse::write(Event& event)
{
#ifndef NDEBUG
    check_valid_pages(map.size());
    check_block_hotness();
    check_ftl_hotness_integrity(event);
#endif

    const ulong lpn = event.get_logical_address();
    PlaneAddress evtPlaneAddr = lpnToPlane(lpn);
    Address& evtHWF = HWF.at(evtPlaneAddr.to_linear_address());
    Address& evtCWF = CWF.at(evtPlaneAddr.to_linear_address());

    wlvl->prewrite(event, controller, {evtHWF, evtCWF}); /// WARNING Returns status, but no action needed on failure

    controller.stats.numFTLWrite++;

    /// Invalidate previous page
    get_block(map[lpn])->invalidate_page(map[lpn].page, event.get_start_time() + event.get_time_taken());

    const bool is_actually_hot = hcID->is_hot(lpn);
    const bool false_result = RandNrGen::get() < (is_actually_hot ? prob_fneg : prob_fpos);
    const bool lpnIsHot = false_result ? !is_actually_hot : is_actually_hot;

    while (get_next_page(evtHWF) != SUCCESS or get_next_page(evtCWF) != SUCCESS) // Still space in WF
    {
        const bool HWFInitiated = get_next_page(evtHWF) != SUCCESS; // HWF initiated cleaning cycle

        // Need to select a victim block through GC
        Address victim = map[lpn];
        const std::vector<Address> currentWF = {evtHWF, evtCWF};

        if (wlvl->suggest_WF(event, victim, controller, currentWF) == FAILURE) {
            this->gca_collect(event, victim, HWFInitiated, currentWF);
        }

        this->merge(event, victim, HWFInitiated, controller.stats);
    }
    if (lpnIsHot) {
        event.set_address(evtHWF); // Tell WF to write to this (next) page
        map[lpn] = evtHWF;
    } else {
        event.set_address(evtCWF); // Tell WF to write to this (next) page
        map[lpn] = evtCWF;
    }

#ifndef NDEBUG
    check_valid_pages(map.size() - 1);
#endif

    return SUCCESS;
}
