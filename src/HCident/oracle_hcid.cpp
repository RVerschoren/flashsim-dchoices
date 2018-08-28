/* Copyright 2017 Robin Verschoren */

/* oracle_hcid.cpp  */

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

/* Implements an oracle-based hot/cold identification technique. */

#include "ssd.h"

using namespace ssd;

// Oracle_HCID::Oracle_HCID(const std::vector<Event> &events)
//{
//    /// TODO Do this for several frames
//    for(const Event &event : events)
//    {
//        if(event.is_hot()) hotSet.insert(event.get_logical_address());
//    }
//}

Oracle_HCID::Oracle_HCID(const std::set<ulong>& hotSet)
	: hotSet(hotSet)
{
}

Oracle_HCID::~Oracle_HCID()
{
}

bool
Oracle_HCID::is_hot(const ulong lpn) const
{
	return hotSet.find(lpn) != hotSet.end();
}
