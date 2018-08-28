/* Copyright 2017 Robin Verschoren */

/* costbenefit_gc.cpp */

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

/* Implements the cost-benefit GC algorithm, proposed by Kawaguchi et al, 1995.
 */

#include "ssd.h"
#include "util.h"
#include <assert.h>

using namespace ssd;

GCImpl_CostBenefit::GCImpl_CostBenefit(FtlParent* ftl)
    : Garbage_collector(ftl)
{
}

GCImpl_CostBenefit::~GCImpl_CostBenefit()
{
}

void
GCImpl_CostBenefit::collect(const Event& evt, Address& victimAddress,
                            const std::function<bool(const Address&)>& ignorePred)
{
	std::function<double(const Address&)> costFunc =
	[this, &evt](const Address& addr) {
		const Block* blockPtr = ftl->get_block(addr);
		const double age =
		    evt.get_start_time() - blockPtr->get_last_erase_time();
		const double u = static_cast<double>(blockPtr->get_pages_valid()) /
		                 static_cast<double>(BLOCK_SIZE);
        const double benefitPerCost = (2.0 * u) / (age * (1.0 - u));
		return benefitPerCost;
	};

	greedy_block(victimAddress, costFunc, ignorePred);
}
