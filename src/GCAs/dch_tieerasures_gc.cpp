/* Copyright 2020 Robin Verschoren */

/* dch_tieerasures_gc.cpp */

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

/* Implements the d-choices GC algorithm (selects a victim block in random
 * fashion), with a tie breaker for the least amount of erasures. */

#include "ssd.h"
#include "util.h"
#include <assert.h>

using namespace ssd;

GCImpl_DCh_TieErasures::GCImpl_DCh_TieErasures(FtlParent* ftl, const uint d) : Garbage_collector(ftl), d(d) {}

GCImpl_DCh_TieErasures::~GCImpl_DCh_TieErasures() {}

void GCImpl_DCh_TieErasures::collect(const Event& /*evt*/, Address& victimAddress,
                                     const std::function<bool(const Address&)>& ignorePred, bool replacingHWF)
{
	/// TODO Re-enable with compile flag to follow the technical procedures
    // d_choices_block_same_plane_tiebreak(d, victimAddress, validPages, eraseCount, ignorePred);
    d_choices_block_same_plane_min_valid_pages_tie_min_erase(ftl->controller, d, victimAddress, ignorePred);
}
