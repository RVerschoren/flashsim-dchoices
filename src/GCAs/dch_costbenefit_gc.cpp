/* Copyright 2018 Robin Verschoren */

/* dch_costbenefit_gc.cpp */

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

/* Implements the cost-benefit GC algorithm on d randomly chosen blocks.
 */

#include "ssd.h"
#include "util.h"
#include <assert.h>

using namespace ssd;

GCImpl_DChoices_CostBenefit::GCImpl_DChoices_CostBenefit(FtlParent* ftl, const uint d)
    : Garbage_collector(ftl)
    , d(d)
{
}

GCImpl_DChoices_CostBenefit::~GCImpl_DChoices_CostBenefit()
{
}

void
GCImpl_DChoices_CostBenefit::collect(const Event& evt, Address& victimAddress,
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
        d_choices_block(d, victimAddress, costFunc, ignorePred);
}
