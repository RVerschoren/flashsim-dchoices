/* Copyright 2017 Robin Verschoren */

/* random_gc.cpp  */

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

/* Implements the random GC algorithm (selects a victim block in random
 * fashion). */

#include "ssd.h"
#include "util.h"
#include <assert.h>

using namespace ssd;

GCImpl_Random::GCImpl_Random(FtlParent* ftl)
	: Garbage_collector(ftl)
{
}

GCImpl_Random::~GCImpl_Random()
{
}

void
GCImpl_Random::collect(const Event& /*event*/, Address& addr,
                       const std::function<bool(const Address&)>& ignorePred)
{
    /*std::function<bool(const Address&)> ignorePred = [this, &doNotPick](
	const Address& addr) {
		return block_is_in_vector(addr, doNotPick);
    };*/
	random_block(addr, ignorePred);
	assert(addr.valid ==
	       PAGE); // addr.valid = PAGE; // Make sure this is the case
}
