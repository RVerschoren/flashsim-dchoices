/* Copyright 2017 Robin Verschoren */

/* warm_hcid.cpp  */

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

/* Implements the Write-Awareness Retention Management (WARM) policy from Y. Luo
 * et al. */

#include "ssd.h"
#include <algorithm>
#include <iterator>
#include <queue>

using namespace ssd;

WARM_HCID::WARM_HCID(const uint maxHotQueueLength, const uint maxCooldownLength)
	: hotQueue(maxHotQueueLength)
	, cooldownWindow(maxCooldownLength)
{
}

WARM_HCID::~WARM_HCID()
{
}

bool
WARM_HCID::is_hot(const ulong lpn) const
{
	const auto it = std::find(hotQueue.begin(), hotQueue.end(), lpn);
	return *it == lpn;
	// return hotSet.find(lpn) != hotSet.end();
}

bool
WARM_HCID::request_lpn(const ulong lpn)
{
	assert(hotQueue.size() <= maxNumHot);
	assert(cooldownWindow.size() <= maxNumCooldown);

	bool lpnIsHot = false;

	const std::deque<ulong>::iterator it =
	    std::find(std::begin(hotQueue), std::end(hotQueue), lpn);
	if (it != std::end(hotQueue)) { // Hot LPN
		assert(*it == lpn);
		hotQueue.erase(it);
		hotQueue.push_front(lpn);
		lpnIsHot = true;
	} else {

		const std::deque<ulong>::iterator it =
		    std::find(std::begin(cooldownWindow), std::end(cooldownWindow), lpn);
		if (it != cooldownWindow.end()) { // LPN in cooldown
			assert(*it == lpn);
			cooldownWindow.erase(it);

			if (hotQueue.size() == maxNumHot) {
				// Remove first, so we stay within capacity
				hotQueue.pop_back();
			}
			hotQueue.push_front(lpn);
			lpnIsHot = true;
		} else {
			if (cooldownWindow.size() == maxNumCooldown) {
				// Remove first, so we stay within capacity
				cooldownWindow.pop_back();
			}
			cooldownWindow.push_front(lpn);
			lpnIsHot = false;
		}
	}
	assert(hotQueue.size() <= maxNumHot);
	assert(cooldownWindow.size() <= maxNumCooldown);
	return lpnIsHot;
}
