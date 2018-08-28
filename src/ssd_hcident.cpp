/* Copyright 2017 Robin Verschoren */

/* ssd_ftlparent.cpp  */

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

/*
 * Implements parent interface for all hot/cold data identification schemes to
 * use.
 */

#include "ssd.h"

using namespace ssd;

bool
HotColdID::is_hot(const ulong /*lpn*/) const
{
	return false;
}

bool
HotColdID::request_lpn(const ulong lpn)
{
	return is_hot(lpn);
}
