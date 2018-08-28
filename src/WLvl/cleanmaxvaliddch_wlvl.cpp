/* Copyright 2017 Robin Verschoren */

/* ban_wlvl.cpp */

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

/* WLvlImpl_CleanMaxValidDCh class
 *
 * Class implementing a greedier version of the Ban wear leveling scheme, which
 * cleans the block with the highest number
 * of valid pages with a probability p.
 */

#include "ssd.h"
#include <assert.h>

using namespace ssd;

WLvlImpl_CleanMaxValidDCh::WLvlImpl_CleanMaxValidDCh(FtlParent* ftl,
        const double p,
        const uint d)
	: Wear_leveler(ftl)
	, p(p)
	, d(d)
{
}

WLvlImpl_CleanMaxValidDCh::~WLvlImpl_CleanMaxValidDCh()
{
}

enum status
WLvlImpl_CleanMaxValidDCh::suggest_WF(Address& WFSuggestion,
                                      const std::vector<Address>& doNotPick) {
	if (RandNrGen::get() < p)   // Pick a random block
	{
		std::function<uint(const Address&)> costFunc =
		[this](const Address& addr) {
			return BLOCK_SIZE - FTL->get_pages_valid(addr);
		};
		std::function<bool(const Address&)> ignorePred = [this, &doNotPick](
		const Address& addr) {
			return block_is_in_vector(addr, doNotPick);
		};
		d_choices_block(d, WFSuggestion, costFunc, ignorePred);
		return SUCCESS;
	}
	return FAILURE;
}
