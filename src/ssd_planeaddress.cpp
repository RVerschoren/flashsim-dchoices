/* Copyright 2019 Robin Verschoren */

/* ssd_planeaddress.cpp is part of FlashSim. */

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

/* PlaneAddress class
 *
 * Support for mapping planes (and thus write frontiers) to addresses.
 */

#include "ssd.h"

using namespace ssd;

/*
using namespace std;
namespace std {
template <>
struct hash<ssd::PlaneAddress>
{
        size_t operator()(ssd::PlaneAddress const& addr) const noexcept
        {
                return std::hash<uint>((addr.package * PACKAGE__SIZE + addr.die) * DIE_SIZE + addr.plane);
        }
};
}*/ /* end namespace std */

PlaneAddress::PlaneAddress(const Address& address) : package(address.package), die(address.die), plane(address.plane) {}

PlaneAddress::PlaneAddress(uint package, uint die, uint plane) : package(package), die(die), plane(plane) {}

uint PlaneAddress::to_linear_address() const { return (package * PACKAGE_SIZE + die) * DIE_SIZE + plane; }
