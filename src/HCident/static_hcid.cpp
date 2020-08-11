/* Copyright 2017 Robin Verschoren */

/* static_hcid.cpp  */

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

/* Implements a static hot/cold identification technique. */

#include "ssd.h"

using namespace ssd;

Static_HCID::Static_HCID(ulong maxLPN, double hotFraction)
	: maxLPN(maxLPN)
	, hotFraction(hotFraction)
	, numHotLPN(static_cast<ulong>(std::floor(hotFraction * maxLPN)))
{
}

Static_HCID::~Static_HCID()
{
}

bool
Static_HCID::is_hot(ulong lpn) const
{
#ifndef NDEBUG
//	std::cout  << "Static HCID check (" << lpn << " < " << numHotLPN <<  "): " << (lpn < numHotLPN) << std::endl;
#endif
	return lpn < numHotLPN;
}
