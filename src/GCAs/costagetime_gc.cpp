/* Copyright 2018 Robin Verschoren */

/* costagetime_gc.cpp */

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

/* Implements the cost-age time GC algorithm.
 */

#include "ssd.h"
#include "util.h"
#include <assert.h>

using namespace ssd;

GCImpl_CostAgeTime::GCImpl_CostAgeTime(FtlParent* ftl)
    : Garbage_collector(ftl)
{
}

GCImpl_CostAgeTime::~GCImpl_CostAgeTime()
{
}

void
GCImpl_CostAgeTime::collect(const Event& evt, Address& victimAddress,
                            const std::function<bool(const Address&)>& ignorePred)
{
	std::function<double(const Address&)> costFunc =
	[this, &evt](const Address& addr) {
		const Block* blockPtr = ftl->get_block(addr);
		const double age =
		    evt.get_start_time() - blockPtr->get_last_erase_time();
		const double u = static_cast<double>(blockPtr->get_pages_valid()) /
		                 static_cast<double>(BLOCK_SIZE);
        const double CT = blockPtr->get_erase_count();
        return ((1.0 - u) * age) / (u * CT); // Largest cost is smallest benefit
	};

	greedy_block(victimAddress, costFunc, ignorePred);
}
