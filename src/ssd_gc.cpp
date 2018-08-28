/* Copyright 2009, 2010 Brendan Tauras */

/* ssd_gc.cpp is part of FlashSim. */

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

/* Garbage_collector class
* Brendan Tauras 2009-11-04
*
* This class is a stub class for the user to use as a template for implementing
* his/her garbage collector scheme.  The garbage collector class was added to
* simplify and modularize the garbage collection in FTL schemes. */

#include "ssd.h"
#include <assert.h>
#include <new>
#include <stdio.h>
#include "util.h"

using namespace ssd;

Garbage_collector::Garbage_collector(FtlParent* ftl)
	: ftl(ftl)
{
	return;
}

Garbage_collector::~Garbage_collector(void)
{
	return;
}

void
Garbage_collector::collect(const Event& evt, Address& address,
                           const std::vector<Address>& doNotPick)
{
    std::function<bool(const Address&)> ignorePred =
        [&doNotPick](const Address &possibleVictim) {
            return block_is_in_vector(possibleVictim, doNotPick);
    };
    return collect(evt, address, ignorePred);
}

void
Garbage_collector::collect(const Event& /*evt*/, Address& /*address*/,
                           const std::function<bool(const Address&)>& /*ignorePred*/)
{
    return;
}
