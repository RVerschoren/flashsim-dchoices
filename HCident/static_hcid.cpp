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

#include <new>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <set>
#include "../ssd.h"

//using LPNCount = std::map<LPN,int>;
typedef std::map<ulong,int> LPNCount;
//using LPNCountIter = std::iterator<std::map<LPN,int> >;
typedef std::map<ulong,int>::iterator LPNCountIter;
typedef std::set<ulong> HotLPNs;

struct LPNNum{
    ulong lpn;
    unsigned int num;
    LPNNum(const ulong &l, const unsigned int numberOfAppearances) : lpn(l), num(numberOfAppearances){}
};

using namespace ssd;

Static_HCID::Static_HCID(ulong maxLPN, double hotFraction)
    : maxLPN(maxLPN), hotFraction(hotFraction), numHotLPN(std::floor(hotFraction*maxLPN))
{
    for(uint i=0; i < maxLPN; i++)
    {
        hotMap[i] = is_hot(i);
    }
}


Static_HCID::~Static_HCID(){}

void Static_HCID::advance_frame(){
    ///@TODO Implement
}

bool Static_HCID::is_hot(ulong lpn, ulong requestNr)
{
    return lpn < std::floor(hotFraction*maxLPN);
}
/*
ulong Static_HCID::get_num_hot()
{
    ///@TODO This is not trim-proof
    return numHotLPN;
}

double  Static_HCID::get_hot_fraction()
{
    return hotFraction;
}*/
