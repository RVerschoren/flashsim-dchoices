
/* Copyright 2017 Robin Verschoren */

/* ssd_util.cpp */

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

/* Utilities and helper functions
 */

#include "ssd.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <vector>

using namespace ssd;

bool ssd::block_is_in_plane_range(const Address& block, const Address& beginBlockAddress,
								  const Address& endBlockAddress)
{
	assert(beginBlockAddress.compare(endBlockAddress) >= PLANE); // In same plane
	if (beginBlockAddress.block <= block.block and block.block <= endBlockAddress.block) {
		return true;
	}
	return false;
}

bool ssd::block_is_in_vector(const Address& block, const std::vector<Address>& vector)
{
	for (const Address& b : vector) {
		if (block.compare(b) >= BLOCK) {
			return true;
		}
	}
	return false;
}

bool ssd::block_is_in_vector(const Block* block, const std::vector<Block*>& vector)
{
	for (const Block* b : vector) {
		if (block == b) {
			return true;
		}
	}
	return false;
}

void ssd::random_block(Address& victimAddress)
{
	assert(victimAddress.check_valid() >= BLOCK);
	Address address(victimAddress);

	address.package = RandNrGen::get(SSD_SIZE);
	address.die = RandNrGen::get(PACKAGE_SIZE);
	address.plane = RandNrGen::get(DIE_SIZE);
	address.block = RandNrGen::get(PLANE_SIZE);
	assert(address.check_valid() >= BLOCK);

	victimAddress = address;
}

void ssd::random_block(Address& victimAddress, const std::vector<Address>& doNotPick)
{
	assert(victimAddress.check_valid() >= BLOCK);
	/// BUG crashes here?
	Address address(0, 0, 0, 0, 0, PAGE);
	do {
		address.package = RandNrGen::get(SSD_SIZE);
		address.die = RandNrGen::get(PACKAGE_SIZE);
		address.plane = RandNrGen::get(DIE_SIZE);
		address.block = RandNrGen::get(PLANE_SIZE);
		assert(address.package < SSD_SIZE);
		assert(address.die < PACKAGE_SIZE);
		assert(address.plane < DIE_SIZE);
		assert(address.block < PLANE_SIZE);
	} while (block_is_in_vector(address, doNotPick));
	victimAddress = address;

#ifndef NDEBUG
	assert(victimAddress.check_valid() >= BLOCK);
	const bool isDNP1 = address.same_block(doNotPick[0]);
	const bool isDNP2 = address.same_block(doNotPick[1]);
	assert(not isDNP1 and not isDNP2);
#endif
}

void ssd::random_block(Address& victimAddress, const std::function<bool(const Address&)>& ignored)
{
	assert(victimAddress.check_valid() >= BLOCK);
	/// BUG crashes here?
	Address address(0, 0, 0, 0, 0, PAGE);
	do {
		address.package = RandNrGen::get(SSD_SIZE);
		address.die = RandNrGen::get(PACKAGE_SIZE);
		address.plane = RandNrGen::get(DIE_SIZE);
		address.block = RandNrGen::get(PLANE_SIZE);
		assert(address.package < SSD_SIZE);
		assert(address.die < PACKAGE_SIZE);
		assert(address.plane < DIE_SIZE);
		assert(address.block < PLANE_SIZE);
#ifndef NDEBUG
		std::cout << address.package << " " << SSD_SIZE << std::endl;
		std::cout << address.die << " " << PACKAGE_SIZE << std::endl;
		std::cout << address.plane << " " << DIE_SIZE << std::endl;
		std::cout << address.block << " " << PLANE_SIZE << std::endl;
#endif

	} while (ignored(address));
	victimAddress = address;
	assert(victimAddress.check_valid() >= BLOCK);
	assert(not ignored(address));
	/// FIXME Remove
	victimAddress.valid = PAGE;
}
/// TODO Remove
/*void ssd::d_choices_block(const unsigned int d, Address& victimAddress, const std::function<uint(const Address&)>&
cost, const std::function<bool(const Address&)>& ignored)
{
	do {
		Address address(victimAddress);

		uint minCost = std::numeric_limits<uint>::max();
		for (uint i = 0; i < d; i++) {

			address.package = RandNrGen::get(SSD_SIZE);
			address.die = RandNrGen::get(PACKAGE_SIZE);
			address.plane = RandNrGen::get(DIE_SIZE);

			address.block = RandNrGen::get(PLANE_SIZE);
			assert(address.check_valid() >= BLOCK);

			const uint blockCost = cost(address);
			if (blockCost < minCost) {
				minCost = blockCost;
				victimAddress = address;
			}
		}
	} while (ignored(victimAddress));
}

void ssd::greedy_block(Address& victimAddress, const std::function<uint(const Address&)>& cost,
					   const std::function<bool(const Address&)>& ignored)
{
	Address address(victimAddress);

	uint minCost = std::numeric_limits<uint>::max();
	for (uint package = 0; package < SSD_SIZE; package++) {
		address.package = package;
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			address.die = die;
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				address.plane = plane;
				assert(address.check_valid() >= PLANE);
				for (uint block = 0; block < PLANE_SIZE; block++) {
					address.block = block;
					assert(address.check_valid() >= BLOCK);
					if (not ignored(address)) { // block_is_in_vector(address,
						// doNotPick)) {
						const uint blockCost = cost(address);
						if (blockCost < minCost) {
							minCost = blockCost;
							victimAddress = address;
						}
					}
				}
			}
		}
	}
	victimAddress.valid = PAGE;
}

void ssd::greedy_block(Address& victimAddress, const std::function<double(const Address&)>& cost,
					   const std::function<bool(const Address&)>& ignored)
{
	Address address(victimAddress);

	double minCost = std::numeric_limits<double>::max();
	for (uint package = 0; package < SSD_SIZE; package++) {
		address.package = package;
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			address.die = die;
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				address.plane = plane;
				assert(address.check_valid() >= PLANE);
				for (uint block = 0; block < PLANE_SIZE; block++) {
					address.block = block;
					assert(address.check_valid() >= BLOCK);
					if (not ignored(address)) {
						const double blockCost = cost(address);
						if (blockCost < minCost) {
							minCost = blockCost;
							victimAddress = address;
						}
					}
				}
			}
		}
	}
	victimAddress.valid = PAGE;
}
*/
void ssd::random_block_same_plane(Address& victimAddress)
{
	assert(victimAddress.check_valid() >= BLOCK);
	Address address(victimAddress);

	address.block = RandNrGen::get(PLANE_SIZE);
	assert(address.check_valid() >= BLOCK);

	victimAddress = address;
}

void ssd::random_block_same_plane(Address& victimAddress, const std::vector<Address>& doNotPick)
{
	assert(victimAddress.check_valid() >= BLOCK);
	/// BUG crashes here?
	Address address(0, 0, 0, 0, 0, PAGE);
	do {
		address.block = RandNrGen::get(PLANE_SIZE);
		assert(address.block < PLANE_SIZE);
	} while (block_is_in_vector(address, doNotPick));
	victimAddress = address;

#ifndef NDEBUG
	assert(victimAddress.check_valid() >= BLOCK);
	const bool isDNP1 = address.same_block(doNotPick[0]);
	const bool isDNP2 = address.same_block(doNotPick[1]);
	assert(not isDNP1 and not isDNP2);
#endif
}

void ssd::random_block_same_plane(Address& victimAddress, const std::function<bool(const Address&)>& ignored)
{
	assert(victimAddress.check_valid() >= BLOCK);
	/// BUG crashes here?
	Address address(0, 0, 0, 0, 0, PAGE);
	do {
		address.block = RandNrGen::get(PLANE_SIZE);
		assert(address.block < PLANE_SIZE);
	} while (ignored(address));
	victimAddress = address;
	assert(victimAddress.check_valid() >= BLOCK);
	assert(not ignored(address));
	/// FIXME Remove
	victimAddress.valid = PAGE;
}

/// TODO Remove
void ssd::d_choices_block_same_plane_min_valid_pages(Controller& ctrl, const unsigned int d, Address& victimAddress,
													 const std::function<bool(const Address&)>& ignored)
{
	ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
	Address address(victimAddress);

	uint minCost = std::numeric_limits<uint>::max();
	auto minCostBlockNr = address.block;
	uint tries = 0;
	while (tries < d) {
#ifndef NDEBUG
		assert(address.check_valid() >= BLOCK);
#endif
		const uint blockNr = RandNrGen::get(PLANE_SIZE);
		const uint blockCost = plane->data[blockNr].pages_valid;
		address.block = blockNr;
		if (not ignored(address)) {
			tries++;
			if (blockCost < minCost) {
				minCost = blockCost;
				minCostBlockNr = blockNr;
			}
		}
	}
	victimAddress.block = minCostBlockNr;
}

void ssd::d_choices_block_same_plane_min_valid_pages_tie_min_erase(Controller& ctrl, const unsigned int d,
                                                                   Address& victimAddress,
                                                                   const std::function<bool(const Address&)>& ignored)
{
    ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
    Address address(victimAddress);

    uint minCost = std::numeric_limits<uint>::max();
    std::vector<uint> minCostBlockNumbers = {};
    ulong minCostErases = std::numeric_limits<uint>::max();

    uint tries = 0;
    while (tries < d) {
#ifndef NDEBUG
        assert(address.check_valid() >= BLOCK);
#endif
        const uint blockNr = RandNrGen::get(PLANE_SIZE);
        const uint blockCost = plane->data[blockNr].pages_valid;
        const ulong blockErases = plane->data[blockNr].get_erase_count();

        address.block = blockNr;
        if (not ignored(address)) {
            tries++;
            const bool lessCost = blockCost < minCost;
            const bool sameCost = blockCost == minCost;
            const bool sameTie = blockErases == minCostErases;
            const bool tiePrefer = blockErases < minCostErases;
            if (sameCost and sameTie) {
                // Select the block with the least amount of erases
                minCostBlockNumbers.push_back(blockNr);
            } else if (lessCost or (sameCost and tiePrefer)) {
                minCostBlockNumbers.clear();
                minCostBlockNumbers.push_back(blockNr);
                minCost = blockCost;
                minCostErases = blockErases;
            }
        }
    }
    victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
}

void ssd::d_choices_block_same_plane_min_valid_pages_tie_max_erase(Controller& ctrl, const unsigned int d,
                                                                   Address& victimAddress,
                                                                   const std::function<bool(const Address&)>& ignored)
{
    ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
    Address address(victimAddress);

    uint minCost = std::numeric_limits<uint>::max();
    std::vector<uint> minCostBlockNumbers = {};
    ulong maxCostErases = std::numeric_limits<uint>::min();

    uint tries = 0;
    while (tries < d) {
#ifndef NDEBUG
        assert(address.check_valid() >= BLOCK);
#endif
        const uint blockNr = RandNrGen::get(PLANE_SIZE);
        const uint blockCost = plane->data[blockNr].pages_valid;
        const ulong blockErases = plane->data[blockNr].get_erase_count();

        address.block = blockNr;
        if (not ignored(address)) {
            tries++;
            const bool lessCost = blockCost < minCost;
            const bool sameCost = blockCost == minCost;
            const bool sameTie = blockErases == maxCostErases;
            const bool tiePrefer = blockErases > maxCostErases;
            if (sameCost and sameTie) {
                // Select the block with the least amount of erases
                minCostBlockNumbers.push_back(blockNr);
            } else if (lessCost or (sameCost and tiePrefer)) {
                minCostBlockNumbers.clear();
                minCostBlockNumbers.push_back(blockNr);
                minCost = blockCost;
                maxCostErases = blockErases;
            }
        }
    }
    victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
}

void ssd::d_choices_block_same_plane(const unsigned int d, Address& victimAddress,
									 const std::function<uint(const Address&)>& cost,
									 const std::function<bool(const Address&)>& ignored)
{
	do {
		Address address(victimAddress);

		uint minCost = std::numeric_limits<uint>::max();
		for (uint i = 0; i < d; i++) {
			address.block = RandNrGen::get(PLANE_SIZE);
#ifndef NDEBUG
			assert(address.check_valid() >= BLOCK);
#endif
			const uint blockCost = cost(address);
			if (blockCost < minCost) {
				minCost = blockCost;
				victimAddress = address;
			}
		}
	} while (ignored(victimAddress));
}

void ssd::d_choices_block_same_plane_min_erase(Controller& ctrl, const unsigned int d, Address& victimAddress,
                                               const std::function<bool(const Address&)>& ignored)
{

    ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
    Address address(victimAddress);

    ulong minCost = std::numeric_limits<uint>::max();
    auto minCostBlockNr = address.block;
    uint tries = 0;
    while (tries < d) {
#ifndef NDEBUG
        assert(address.check_valid() >= BLOCK);
#endif
        const uint blockNr = RandNrGen::get(PLANE_SIZE);
        const ulong blockCost = plane->data[blockNr].get_erase_count();
        address.block = blockNr;
        if (not ignored(address)) {
            tries++;
            if (blockCost < minCost) {
                minCost = blockCost;
                minCostBlockNr = blockNr;
            }
        }
    }
    victimAddress.block = minCostBlockNr;
}

void ssd::d_choices_block_same_plane_min_erase_tie_valid(Controller& ctrl, const unsigned int d, Address& victimAddress,
                                                         const std::function<bool(const Address&)>& ignored)
{

    ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
    Address address(victimAddress);

    ulong minCost = std::numeric_limits<ulong>::max();
    std::vector<uint> minCostBlockNumbers = {};
    ulong minCostTie = std::numeric_limits<ulong>::max();

    uint tries = 0;
    while (tries < d) {
#ifndef NDEBUG
        assert(address.check_valid() >= BLOCK);
#endif
        const uint blockNr = RandNrGen::get(PLANE_SIZE);
        const ulong blockCost = plane->data[blockNr].get_erase_count();
        const ulong blockTieCost = plane->data[blockNr].pages_valid;

        address.block = blockNr;
        if (not ignored(address)) {
            tries++;
            if ((blockCost < minCost) or (blockCost == minCost and blockTieCost < minCostTie)) {
                minCostBlockNumbers.clear();
                minCostBlockNumbers.push_back(blockNr);
                minCost = blockCost;
                minCostTie = blockTieCost;
            } else if (blockCost == minCost and blockTieCost == minCostTie) {
                // Select the block with the least amount of valid pages
                minCostBlockNumbers.push_back(blockNr);
            }
        }
    }
    victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
}

void ssd::greedy_block_same_plane(Address& victimAddress, const std::function<uint(const Address&)>& cost,
								  const std::function<bool(const Address&)>& ignored)
{
	Address address(victimAddress);

	uint minCost = std::numeric_limits<uint>::max();
	assert(address.check_valid() >= PLANE);
	for (uint block = 0; block < PLANE_SIZE; block++) {
		address.block = block;
		assert(address.check_valid() >= BLOCK);
		if (not ignored(address)) {
			const uint blockCost = cost(address);
			if (blockCost < minCost) {
				minCost = blockCost;
				victimAddress = address;
			}
		}
	}

	victimAddress.valid = PAGE;
}

void ssd::greedy_block_same_plane(Address& victimAddress, const std::function<double(const Address&)>& cost,
								  const std::function<bool(const Address&)>& ignored)
{
	Address address(victimAddress);

	double minCost = std::numeric_limits<double>::max();
	assert(address.check_valid() >= PLANE);
	for (uint block = 0; block < PLANE_SIZE; block++) {
		address.block = block;
		assert(address.check_valid() >= BLOCK);
		if (not ignored(address)) {
			const double blockCost = cost(address);
			if (blockCost < minCost) {
				minCost = blockCost;
				victimAddress = address;
			}
		}
	}
	victimAddress.valid = PAGE;
}

void ssd::greedy_block_same_plane_min_valid_pages(Controller& ctrl, Address& victimAddress,
												  const std::function<bool(const Address&)>& ignored)
{
    /** Corrected: pick the minimum block
        if a block is encountered with the same number of valid pages, decide whether to change or not
        This should lead to equal probability to get chosen for all blocks with the same amount of valid pages... *
	ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
	Address address(victimAddress);

	uint minCost = std::numeric_limits<uint>::max();
	auto minCostBlockNr = address.block;
	auto minCounter = 1;
	for (auto blockNr = 0; blockNr < PLANE_SIZE; blockNr++) {
		const uint blockCost = plane->data[blockNr].pages_valid;
		address.block = blockNr;
		if (not ignored(address)) {

			if (blockCost < minCost) {
				minCounter = 1;
				minCost = blockCost;
				minCostBlockNr = blockNr;
			} else if (blockCost == minCost) {
				minCounter++;
				if (RandNrGen::get() < 1.0 / minCounter) {
					minCostBlockNr = blockNr;
				}
			}
		}
	}
    victimAddress.block = minCostBlockNr;
    */
    ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
    Address address(victimAddress);

    uint minCost = std::numeric_limits<uint>::max();
    std::vector<ulong> minCostBlockNumbers = {};
    for (auto blockNr = 0; blockNr < PLANE_SIZE; blockNr++) {
        const uint blockCost = plane->data[blockNr].pages_valid;
        address.block = blockNr;
        if (not ignored(address)) {
            if (blockCost < minCost) {
                minCostBlockNumbers.clear();
                minCostBlockNumbers.push_back(blockNr);
                minCost = blockCost;
            } else if (blockCost == minCost) {
                minCostBlockNumbers.push_back(blockNr);
            }
        }
    }
    victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
}

void ssd::greedy_block_same_plane_min_valid_pages_tie_min_erase(Controller& ctrl, Address& victimAddress,
                                                                const std::function<bool(const Address&)>& ignored)
{
	ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
	Address address(victimAddress);

	uint minCost = std::numeric_limits<uint>::max();
    std::vector<uint> minCostBlockNumbers = {};
    ulong minCostErases = std::numeric_limits<uint>::max();
	for (auto blockNr = 0; blockNr < PLANE_SIZE; blockNr++) {
		const uint blockCost = plane->data[blockNr].pages_valid;
        const ulong blockErases = plane->data[blockNr].get_erase_count();
        address.block = blockNr;
		if (not ignored(address)) {
            const bool lessCost = blockCost < minCost;
            const bool sameCost = blockCost == minCost;
            const bool sameTie = blockErases == minCostErases;
            const bool tiePrefer = blockErases < minCostErases;
            if (sameCost and sameTie) {
                // Select the block with the least amount of erases
                minCostBlockNumbers.push_back(blockNr);
            } else if (lessCost or (sameCost and tiePrefer)) {
				minCostBlockNumbers.clear();
				minCostBlockNumbers.push_back(blockNr);
				minCost = blockCost;
                minCostErases = blockErases;
			}
		}
	}
	victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
}

void ssd::greedy_block_same_plane_min_valid_pages_tie_max_erase(Controller& ctrl, Address& victimAddress,
                                                                const std::function<bool(const Address&)>& ignored)
{
    ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
    Address address(victimAddress);

    uint minCost = std::numeric_limits<uint>::max();
    std::vector<uint> minCostBlockNumbers = {};
    ulong maxCostErases = std::numeric_limits<uint>::min();
    for (auto blockNr = 0; blockNr < PLANE_SIZE; blockNr++) {
        const uint blockCost = plane->data[blockNr].pages_valid;
        const ulong blockErases = plane->data[blockNr].get_erase_count();
        address.block = blockNr;
        if (not ignored(address)) {
            const bool lessCost = blockCost < minCost;
            const bool sameCost = blockCost == minCost;
            const bool sameTie = blockErases == maxCostErases;
            const bool tiePrefer = blockErases > maxCostErases;
            if (sameCost and sameTie) {
                // Select the block with the least amount of erases
                minCostBlockNumbers.push_back(blockNr);
            } else if (lessCost or (sameCost and tiePrefer)) {
                minCostBlockNumbers.clear();
                minCostBlockNumbers.push_back(blockNr);
                minCost = blockCost;
                maxCostErases = blockErases;
            }
        }
    }
    victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
}

void ssd::greedy_block_same_plane_min_erase(Controller& ctrl, Address& victimAddress,
                                            const std::function<bool(const Address&)>& ignored)
{
    ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
    Address address(victimAddress);

    uint minCost = std::numeric_limits<uint>::max();
    std::vector<ulong> minCostBlockNumbers = {};
    for (auto blockNr = 0; blockNr < PLANE_SIZE; blockNr++) {
        const uint blockCost = plane->data[blockNr].get_erase_count();
        address.block = blockNr;
        if (not ignored(address)) {
            if (blockCost < minCost) {
                minCostBlockNumbers.clear();
                minCostBlockNumbers.push_back(blockNr);
                minCost = blockCost;
            } else if (blockCost == minCost) {
                minCostBlockNumbers.push_back(blockNr);
            }
        }
    }
    victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
}

void ssd::greedy_block_same_plane_min_erase_tie_valid(Controller& ctrl, Address& victimAddress,
                                                      const std::function<bool(const Address&)>& ignored)
{
    ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
    Address address(victimAddress);

    ulong minCost = std::numeric_limits<ulong>::max();
    std::vector<uint> minCostBlockNumbers = {};
    ulong minCostTie = std::numeric_limits<ulong>::max();
    for (auto blockNr = 0; blockNr < PLANE_SIZE; blockNr++) {
        const ulong blockCost = plane->data[blockNr].get_erase_count();
        const ulong blockTie = plane->data[blockNr].pages_valid;
        address.block = blockNr;
        if (not ignored(address)) {
            if ((blockCost < minCost) or (blockCost == minCost and blockTie < minCostTie)) {
                minCostBlockNumbers.clear();
                minCostBlockNumbers.push_back(blockNr);
                minCost = blockCost;
                minCostTie = blockTie;
            } else if (blockCost == minCost and blockTie == minCostTie) {
                // Select the block with the least amount of valid pages
                minCostBlockNumbers.push_back(blockNr);
            }
        }
    }
    victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
}
