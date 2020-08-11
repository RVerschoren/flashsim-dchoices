/* Copyright 2018 Robin Verschoren */

/* static_false_hcid.cpp  */

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

/* Implements a static hot/cold identification technique with support for (uniform) random false positives/negatives. */

#include "ssd.h"
#include "util.h"

using namespace ssd;

Static_False_HCID::Static_False_HCID(ulong maxLPN, double false_pos_prob, double false_neg_prob, double hotFraction)
	: maxLPN(maxLPN)
    , fp(false_pos_prob)
    , fn(false_neg_prob)
	, hotFraction(hotFraction)
	, numHotLPN(static_cast<ulong>(std::floor(hotFraction * maxLPN)))
{
}

Static_False_HCID::~Static_False_HCID()
{
}

bool
Static_False_HCID::is_hot(ulong lpn) const
{
    const bool is_actually_hot = lpn < numHotLPN;
    const bool false_result = RandNrGen::get() < (is_actually_hot? fn : fp);
    return false_result? !is_actually_hot : is_actually_hot;
}
