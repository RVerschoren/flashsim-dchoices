/* Copyright 2017 Robin Verschoren */

/* util_rng.cpp*/

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

/* Utility class for generating pseudorandom numbers.
 * 2017 Robin Verschoren
 */

#include "../util.h"


RandNrGen::RandNrGen(){
    uint32_t seedVal = time(NULL);
    rng.seed(seedVal);
}

RandNrGen& RandNrGen::getInstance()
{
    // Magic static!
    static RandNrGen instance;
    return instance;
}

void RandNrGen::reset(uint32_t seed){
    rng.seed(seed);
}


double RandNrGen::get()
{
    return RandNrGen::dist(RandNrGen::rng);
}

unsigned long RandNrGen::get(unsigned long S)
{
    return std::floor(S*dist(rng));
}

unsigned int RandNrGen::get(unsigned int S)
{
    return std::floor(S*dist(rng));
}
