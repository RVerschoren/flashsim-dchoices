/* Copyright 2017 Robin Verschoren */

/* dchoices_gc.cpp */

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
 * fashion). */

#include "ssd.h"
#include "util.h"
#include <assert.h>

using namespace ssd;

GCImpl_DChoices::GCImpl_DChoices(FtlParent* ftl, const uint d) : Garbage_collector(ftl), d(d) {}

GCImpl_DChoices::~GCImpl_DChoices() {}

void GCImpl_DChoices::collect(const Event& /*evt*/, Address& victimAddress,
                              const std::function<bool(const Address&)>& ignorePred, bool replacingHotBlock)
{
    std::function<uint(const Address&)> validPages = [this](const Address& addr) { return ftl->get_pages_valid(addr); };
    d_choices_block_same_plane(d, victimAddress, validPages, ignorePred);
}
