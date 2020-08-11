/* Copyright 2020 Robin Verschoren */

/* dch_hotcold_gc.cpp */

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
 * fashion). If we are replacing a hot write frontier, look for a cold block with minimum erase count.
 * If replacing a cold write frontier, look for a hot block with maximum erase count. */

#include "ssd.h"
#include "util.h"
#include <assert.h>

using namespace ssd;

GCImpl_DCh_HotCold::GCImpl_DCh_HotCold(FtlParent* ftl, const uint d) : Garbage_collector(ftl), d(d) {}

GCImpl_DCh_HotCold::~GCImpl_DCh_HotCold() {}

void GCImpl_DCh_HotCold::collect(const Event& /*evt*/, Address& victimAddress,
                              const std::function<bool(const Address&)>& ignorePred, bool replacingHotBlock)
{
    if(replacingHotBlock){
        std::function<bool (const Address&)> ignoreHot = [this, &ignorePred](const Address& addr){return ftl->get_block_hotness(addr) || ignorePred(addr);};
        d_choices_block_same_plane_min_valid_pages_tie_min_erase(ftl->controller,d,victimAddress,ignoreHot);
    }else{
        std::function<bool (const Address&)> ignoreCold = [this, &ignorePred](const Address& addr){return (!ftl->get_block_hotness(addr)) || ignorePred(addr);};
        d_choices_block_same_plane_min_valid_pages_tie_max_erase(ftl->controller,d,victimAddress,ignoreCold);
    }
}
