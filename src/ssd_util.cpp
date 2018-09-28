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
#include <limits>
#include <boost/algorithm/string.hpp>

using namespace ssd;

bool
ssd::block_is_in_plane_range(const Address& block,
                        const Address& beginBlockAddress, const Address& endBlockAddress)
{
	assert(beginBlockAddress.compare(endBlockAddress) >= PLANE); // In same plane
	if (beginBlockAddress.block <= block.block and block.block <= endBlockAddress.block) {
		return true;
	}
	return false;
}

bool
ssd::block_is_in_vector(const Address& block,
                        const std::vector<Address>& vector)
{
	for (const Address& b : vector) {
		if (block.compare(b) >= BLOCK) {
			return true;
		}
	}
	return false;
}

bool
ssd::block_is_in_vector(const Block* block, const std::vector<Block*>& vector)
{
	for (const Block* b : vector) {
		if (block == b) {
			return true;
		}
	}
	return false;
}

void
ssd::random_block(Address& victimAddress)
{
	assert(victimAddress.check_valid() >= BLOCK);
	Address address(victimAddress);

#ifndef SINGLE_PLANE
	address.package = RandNrGen::get(SSD_SIZE);
	address.die = RandNrGen::get(PACKAGE_SIZE);
	address.plane = RandNrGen::get(DIE_SIZE);
#else
	address.package = 0;
	address.die = 0;
	address.plane = 0;
#endif
	address.block = RandNrGen::get(PLANE_SIZE);
	assert(address.check_valid() >= BLOCK);

	victimAddress = address;
}

void
ssd::random_block(Address& victimAddress, const std::vector<Address>& doNotPick)
{
    assert(victimAddress.check_valid() >= BLOCK);
	/// TODO crashes here?
	Address address(0, 0, 0, 0, 0, PAGE);
	do {
#ifndef SINGLE_PLANE
		address.package = RandNrGen::get(SSD_SIZE);
		address.die = RandNrGen::get(PACKAGE_SIZE);
		address.plane = RandNrGen::get(DIE_SIZE);
#else
		address.package = 0;
		address.die = 0;
		address.plane = 0;
#endif
		address.block = RandNrGen::get(PLANE_SIZE);
		assert(address.package < SSD_SIZE);
		assert(address.die < PACKAGE_SIZE);
		assert(address.plane < DIE_SIZE);
		assert(address.block < PLANE_SIZE);
#ifdef DEBUG
		std::cout << address.package << " " << SSD_SIZE << std::endl;
		std::cout << address.die << " " << PACKAGE_SIZE << std::endl;
		std::cout << address.plane << " " << DIE_SIZE << std::endl;
		std::cout << address.block << " " << PLANE_SIZE << std::endl;
#endif

	} while (block_is_in_vector(address, doNotPick));
	victimAddress = address;

#ifdef DEBUG
	assert(victimAddress.check_valid() >= BLOCK);
	const bool isDNP1 = address.same_block(doNotPick[0]);
	const bool isDNP2 = address.same_block(doNotPick[1]);
	assert(not isDNP1 and not isDNP2);
#endif
}

void
ssd::random_block(Address& victimAddress,
                  const std::function<bool(const Address&)>& ignored)
{
    assert(victimAddress.check_valid() >= BLOCK);
	/// TODO crashes here?
	Address address(0, 0, 0, 0, 0, PAGE);
	do {
#ifndef SINGLE_PLANE
		address.package = RandNrGen::get(SSD_SIZE);
		address.die = RandNrGen::get(PACKAGE_SIZE);
		address.plane = RandNrGen::get(DIE_SIZE);
#else
		address.package = 0;
		address.die = 0;
		address.plane = 0;
#endif
		address.block = RandNrGen::get(PLANE_SIZE);
		assert(address.package < SSD_SIZE);
		assert(address.die < PACKAGE_SIZE);
		assert(address.plane < DIE_SIZE);
		assert(address.block < PLANE_SIZE);
#ifdef DEBUG
		std::cout << address.package << " " << SSD_SIZE << std::endl;
		std::cout << address.die << " " << PACKAGE_SIZE << std::endl;
		std::cout << address.plane << " " << DIE_SIZE << std::endl;
		std::cout << address.block << " " << PLANE_SIZE << std::endl;
#endif

	} while (ignored(address));
	victimAddress = address;
	assert(victimAddress.check_valid() >= BLOCK);
	assert(not ignored(address));
	/// TODO Remove
	victimAddress.valid = PAGE;
}

void
ssd::d_choices_block(const unsigned int d, Address& victimAddress,
                     const std::function<uint(const Address&)>& cost,
                     const std::function<bool(const Address&)>& ignored)
{
    do {
		Address address(victimAddress);

		uint minCost = std::numeric_limits<uint>::max();
		for (uint i = 0; i < d; i++) {
#ifndef SINGLE_PLANE
			address.package = RandNrGen::get(SSD_SIZE);
			address.die = RandNrGen::get(PACKAGE_SIZE);
			address.plane = RandNrGen::get(DIE_SIZE);
#else
			address.package = 0;
			address.die = 0;
			address.plane = 0;
#endif
			address.block = RandNrGen::get(PLANE_SIZE);
			assert(address.check_valid() >= BLOCK);

			const uint blockCost = cost(address);
            if (blockCost < minCost) {
				minCost = blockCost;
				victimAddress = address;
			}
		}
    } while (ignored(victimAddress));
/*
    bool foundNotIgnored = false;
    do {
        Address address(victimAddress);
        std::vector<Address> possibleVictims(d);
        uint minCost = std::numeric_limits<uint>::max();
        for (uint i = 0; i < d; i++) {
#ifndef SINGLE_PLANE
            address.package = RandNrGen::get(SSD_SIZE);
            address.die = RandNrGen::get(PACKAGE_SIZE);
            address.plane = RandNrGen::get(DIE_SIZE);
#else
            address.package = 0;
            address.die = 0;
            address.plane = 0;
#endif
            address.block = RandNrGen::get(PLANE_SIZE);
            assert(address.check_valid() >= BLOCK);
            possibleVictims[i] = address;
        }
        for(const Address& posVic : possibleVictims){
            if(not ignored(posVic)){
                foundNotIgnored = true;
                const uint blockCost = cost(posVic);
                if (blockCost < minCost){
                    minCost = blockCost;
                    victimAddress = posVic;
                }
            }
        }
    } while (not foundNotIgnored);*/
}

void
ssd::greedy_block(Address& victimAddress,
                  const std::function<uint(const Address&)>& cost,
                  const std::function<bool(const Address&)>& ignored)
{
	Address address(victimAddress);

	uint minCost = std::numeric_limits<uint>::max();
#ifndef SINGLE_PLANE
	for (uint package = 0; package < SSD_SIZE; package++) {
		address.package = package;
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			address.die = die;
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				address.plane = plane;
				assert(address.check_valid() >= PLANE);
#endif
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
#ifndef SINGLE_PLANE
			}
		}
	}
#endif
	victimAddress.valid = PAGE;
}

void
ssd::greedy_block(Address& victimAddress,
                  const std::function<double(const Address&)>& cost,
                  const std::function<bool(const Address&)>& ignored)
{
	Address address(victimAddress);

	double minCost = std::numeric_limits<double>::max();
#ifndef SINGLE_PLANE
	for (uint package = 0; package < SSD_SIZE; package++) {
		address.package = package;
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			address.die = die;
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				address.plane = plane;
				assert(address.check_valid() >= PLANE);
#endif
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
#ifndef SINGLE_PLANE
			}
		}
	}
#endif
	victimAddress.valid = PAGE;
}
/*
std::string ssd::read_sim_parameters(uint argc, char* argv[]){
	uint argit = 1;
	const std::string ftl(argv[argit++]);
	BLOCK_SIZE = static_cast<uint>(std::stoi(argv[argit++]));
	DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
	SPARE_FACTOR = std::stod(argv[argit++]);

	std::string ftlstr;
	if (boost::iequals(ftl, "DWF")) {
		ftlstr = "dwf";
		FTL_IMPLEMENTATION = IMPL_DWF;
		HOT_FRACTION = std::stod(argv[argit++]);      // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
	} else if (boost::iequals(ftl, "HCWF")) {
		ftlstr = "hcwf";
		FTL_IMPLEMENTATION = IMPL_HCWF;
		HOT_FRACTION = std::stod(argv[argit++]);      // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
	} else if (boost::iequals(ftl, "COLD")) {
		ftlstr = "cold";
		FTL_IMPLEMENTATION = IMPL_COLD;
		HOT_FRACTION = std::stod(argv[argit++]);      // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
	} else {
		// SWF
		ftlstr = "swf";
		FTL_IMPLEMENTATION = IMPL_SWF;
	}

	STARTRUN = static_cast<uint>(std::stoi(argv[argit++])); // startrun
	NRUNS = static_cast<uint>(std::stoi(argv[argit++]));    // Number of runs
	PLANE_SIZE = std::stoi(argv[argit++]);
	BLOCK_ERASES = 10000000;
	MAXPE = BLOCK_ERASES - static_cast<uint>(std::stoi(argv[argit++])); // Maximum number of PE cycles, counting back
	return ftlstr;
}


std::string ssd::read_sim_trace_parameters(uint argc, char* argv[], std::string &traceID){
	uint argit = 1;
	const std::string ftl(argv[argit++]);
	BLOCK_SIZE = static_cast<uint>(std::stoi(argv[argit++]));
	DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
	SPARE_FACTOR = std::stod(argv[argit++]);

	std::string ftlstr;
	if (boost::iequals(ftl, "DWF")) {
		ftlstr = "dwf";
		FTL_IMPLEMENTATION = IMPL_DWF;
		HOT_FRACTION = std::stod(argv[argit++]); // f
	} else if (boost::iequals(ftl, "HCWF")) {
		ftlstr = "hcwf";
		FTL_IMPLEMENTATION = IMPL_HCWF;
		HOT_FRACTION = std::stod(argv[argit++]); // f
	} else if (boost::iequals(ftl, "COLD")) {
		ftlstr = "cold";
		FTL_IMPLEMENTATION = IMPL_COLD;
		HOT_FRACTION = std::stod(argv[argit++]); // f
	} else {
		// SWF
		ftlstr = "swf";
		FTL_IMPLEMENTATION = IMPL_SWF;
	}

	STARTRUN = static_cast<uint>(std::stoi(argv[argit++])); // startrun
	NRUNS = static_cast<uint>(std::stoi(argv[argit++]));    // Number of runs
	BLOCK_ERASES = 10000000;
	MAXPE = BLOCK_ERASES -
	        static_cast<uint>(std::stoi(
	                              argv[argit++])); // Maximum number of PE cycles, counting back
	NUMREQUESTS = std::stoul(argv[argit++]);
	TRACEFILE = argv[argit++];
	const std::string traceID = TRACEFILE.substr(0, 4);

	switch (std::stoi(argv[argit++])) {
	case 0:
		READER_MODE = EVTRDR_SIMPLE;
		break;
	case 1:
		READER_MODE = EVTRDR_BIOTRACER;
		break;
	default:
		READER_MODE = EVTRDR_SIMPLE;
		break;
	}
	return ftlstr;
}*/
