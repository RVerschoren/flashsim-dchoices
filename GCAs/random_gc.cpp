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

/* Implements the random GC algorithm (selects a victim block in random fashion). */

//#include <new>
#include <assert.h>
//#include <stdio.h>
//#include <math.h>
#include "../ssd.h"
#include "../util.h"

using namespace ssd;

GCImpl_Random::GCImpl_Random(FtlParent *ftl):
    Garbage_collector(ftl)
{
}

GCImpl_Random::~GCImpl_Random()
{
}

void GCImpl_Random::collect(Address &addr)
{
    addr.package = RandNrGen::getInstance().get(SSD_SIZE);
    addr.die = RandNrGen::getInstance().get(PACKAGE_SIZE);
    addr.plane = RandNrGen::getInstance().get(DIE_SIZE);
    addr.block = RandNrGen::getInstance().get(PLANE_SIZE);
    addr.valid = PAGE;//Make sure this is the case
}
