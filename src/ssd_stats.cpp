/* Copyright 2011 Matias Bjørling */

/* dftp_ftl.cpp  */

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

/* Runtime information for the SSD Model
 */

#include "ssd.h"
#include <assert.h>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

using namespace ssd;

Stats::Stats() { reset(); }

void Stats::reset()
{
	// FTL
	numFTLRead = 0;
	numFTLWrite = 0;
	numFTLErase = 0;
	numFTLTrim = 0;

	// GC
	numGCRead = 0;
	numGCWrite = 0;
	numGCErase = 0;

	// WL
	numWLRead = 0;
	numWLWrite = 0;
	numWLErase = 0;

	// Log based FTL's
	numLogMergeSwitch = 0;
	numLogMergePartial = 0;
	numLogMergeFull = 0;

	// Page based FTL's
	numPageBlockToPageConversion = 0;

	// Cache based FTL's
	numCacheHits = 0;
	numCacheFaults = 0;

	// Possible SSD performance measures
	currentPE = BLOCK_ERASES;
	numCurrentPE = 0;
	thresholdCurrentPE = MAXPE_THRESHOLD;

	PEfairness.clear();
	PEfairness.push_back(0.0); // Initial data point
	SSDendurance.clear();
	SSDendurance.push_back(0.0); // Initial data point
	PEDistribution.clear();

	///@TODO Explanation
	// Number of blocks with at least <index> PE cycles.
	numBlocksPEcycles.clear();
	numBlocksPEcycles.assign(MAX_PE + 1, 0);
	numBlocksPEcycles[0] = PLANE_SIZE * DIE_SIZE * PACKAGE_SIZE * SSD_SIZE;

	// DWF
	WFEHotPagesDist.clear();
	for (uint i = 0; i <= BLOCK_SIZE; i++) {
		WFEHotPagesDist.push_back(0);
	}
	WFIHotPagesDist.clear();
	for (uint i = 0; i <= BLOCK_SIZE; i++) {
		WFIHotPagesDist.push_back(0);
	}
	victimValidDist.clear();
	for (uint i = 0; i <= BLOCK_SIZE; i++) {
		victimValidDist.push_back(0);
	}

	// HCWF
	hotValidPages.clear();
	coldValidPages.clear();
	hotBlocks.clear();
	coldBlocks.clear();
	numHotVictimBlocks.clear();
	numColdVictimBlocks.clear();
	firstColdErase = std::numeric_limits<uint>::infinity();
	hotVictim_ValidDist.clear();
	for (uint i = 0; i <= BLOCK_SIZE; i++) {
		hotVictim_ValidDist.push_back(0);
	}
	coldVictim_ValidDist.clear();
	for (uint i = 0; i <= BLOCK_SIZE; i++) {
		coldVictim_ValidDist.push_back(0);
	}
	hotVictim_HotValidDist.clear();
	for (uint i = 0; i <= BLOCK_SIZE; i++) {
		hotVictim_HotValidDist.push_back(0);
	}
	coldVictim_HotValidDist.clear();
	for (uint i = 0; i <= BLOCK_SIZE; i++) {
		coldVictim_HotValidDist.push_back(0);
	}
	hotGCChainLength.clear();
	hotGCChainLength.push_back(0);
	coldGCChainLength.clear();
	coldGCChainLength.push_back(0);
	numExternalGCInvocations = 0;

	// Swap
	nWLSwaps = 0;
	numWLSwaps.clear();
	numWLSwaps.push_back(nWLSwaps);
	WLSwapCost.clear();
	for (uint i = 0; i <= (2 * BLOCK_SIZE); i++) {
		WLSwapCost.push_back(0);
	}
	nFTLSwaps = 0;
	numFTLSwaps.clear();
	numFTLSwaps.push_back(nWLSwaps);
	FTLSwapCost.clear();
	for (uint i = 0; i <= (2 * BLOCK_SIZE); i++) {
		FTLSwapCost.push_back(0);
	}

	// Memory consumptions (Bytes)
	numMemoryTranslation = 0;
	numMemoryCache = 0;

	numMemoryRead = 0;
	numMemoryWrite = 0;
}

bool ftl_supports_hotcold_blocks()
{
	/// TODO Make FTL-method to determine capabilities?
    return FTL_IMPLEMENTATION == IMPL_HCWF || FTL_IMPLEMENTATION == IMPL_HCWF_FALSE ||
           FTL_IMPLEMENTATION == IMPL_HCWFWDIFF || FTL_IMPLEMENTATION == IMPL_HCWF_FIXED_FALSE ||
           FTL_IMPLEMENTATION == IMPL_HCWF_DHC || FTL_IMPLEMENTATION == IMPL_HCWFPLUSSWAP ||
           FTL_IMPLEMENTATION == IMPL_HCSWAPWF_HOTCOLDTIE || FTL_IMPLEMENTATION == IMPL_HCSWAPWF ||
           FTL_IMPLEMENTATION == IMPL_HCSWAPWF_ERASETIE || FTL_IMPLEMENTATION == IMPL_HCSWAPWF_ERASE ||
           FTL_IMPLEMENTATION == IMPL_HCSWAPWF_ERASE_VALIDTIE || FTL_IMPLEMENTATION == IMPL_COLD ||
           FTL_IMPLEMENTATION == IMPL_STAT || FTL_IMPLEMENTATION == IMPL_DSWAPWF;
}

bool ftl_supports_swap_info()
{
    return FTL_IMPLEMENTATION == IMPL_DWFWDIFF || FTL_IMPLEMENTATION == IMPL_HCWFWDIFF ||
           FTL_IMPLEMENTATION == IMPL_HCSWAPWF || FTL_IMPLEMENTATION == IMPL_HCWFPLUSSWAP ||
           FTL_IMPLEMENTATION == IMPL_HCSWAPWF_HOTCOLDTIE || FTL_IMPLEMENTATION == IMPL_HCSWAPWF_ERASETIE ||
           FTL_IMPLEMENTATION == IMPL_HCSWAPWF_ERASE || FTL_IMPLEMENTATION == IMPL_HCSWAPWF_ERASE_VALIDTIE ||
           FTL_IMPLEMENTATION == IMPL_DSWAPWF;
}

bool wl_supports_swap_info()
{
    return WEAR_LEVELER == WL_RANDOMSWAP || WEAR_LEVELER == WL_HOTCOLDSWAP || WEAR_LEVELER == WL_HOTCOLDSWAP_RELATIVE ||
           WEAR_LEVELER == WL_HOTCOLDGCSWAP || WEAR_LEVELER == WL_HOTCOLDGCSWAP_ERASE;
}

void Stats::erase_block(const uint j, const ulong remainingErases, const bool isHotVictim, const uint k,
						const ulong numHotBlocks)
{
	assert(j <= BLOCK_SIZE);
	numGCRead += j;
	numGCWrite += j;
	numGCErase += 1;
	victimValidDist[j] = victimValidDist[j] + 1;
	if (ftl_supports_hotcold_blocks()) {
		if (isHotVictim) {
			numHotVictims++;
			hotVictim_ValidDist[j] = hotVictim_ValidDist[j] + 1;
			hotVictim_HotValidDist[k] = hotVictim_HotValidDist[k] + 1;
		} else {
			numColdVictims++;
			coldVictim_ValidDist[j] = coldVictim_ValidDist[j] + 1;
			coldVictim_HotValidDist[k] = coldVictim_HotValidDist[k] + 1;
		}
	}
	if (not isHotVictim and firstColdErase == std::numeric_limits<uint>::infinity()) {
		firstColdErase = numGCErase;
	}
	this->numHotBlocks = numHotBlocks;
	this->numColdBlocks = (SSD_SIZE * PACKAGE_SIZE * DIE_SIZE * PLANE_SIZE) - numHotBlocks;

	///@TODO Fix
	const ulong numErases = BLOCK_ERASES - remainingErases;
	if (numErases < numBlocksPEcycles.size()) {
		numBlocksPEcycles[numErases]++;
	}
	while (numBlocksPEcycles[BLOCK_ERASES - this->get_currentPE()] >= thresholdCurrentPE) {
		this->next_currentPE();
	}
	// if (this->get_currentPE() >= remainingErases) {
	//    this->next_currentPE();
	//}
}

void Stats::swap_blocks(const uint totalValidPages, uint numBlocksErased)
{
	numWLRead += totalValidPages;
	numWLWrite += totalValidPages;
	numWLErase += numBlocksErased;
	nWLSwaps += 1;
	if (totalValidPages < WLSwapCost.size())
		WLSwapCost[totalValidPages] += 1;
}
/// NOTE Remove dead code
/// void
/// Stats::copy_after_erase_block(const uint j)
///{
///    numGCWrite += j;
///}

void Stats::reset_statistics() { reset(); }

void Stats::write_header(FILE* stream)
{
	fprintf(stream, "numFTLRead;numFTLWrite;numFTLErase;numFTLTrim;numGCRead;"
					"numGCWrite;numGCErase;numWLRead;numWLWrite;numWLErase;"
					"numLogMergeSwitch;numLogMergePartial;numLogMergeFull;"
					"numPageBlockToPageConversion;numCacheHits;numCacheFaults;"
					"numMemoryTranslation;numMemoryCache;numMemoryRead;"
					"numMemoryWrite\n");
}

void Stats::write_statistics(FILE* stream)
{
	fprintf(stream,
			"%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%"
			"li;%li;%li;%li;%li;%li;\n",
			numFTLRead, numFTLWrite, numFTLErase, numFTLTrim, numGCRead, numGCWrite, numGCErase, numWLRead, numWLWrite,
			numWLErase, numLogMergeSwitch, numLogMergePartial, numLogMergeFull, numPageBlockToPageConversion,
			numCacheHits, numCacheFaults, numMemoryTranslation, numMemoryCache, numMemoryRead, numMemoryWrite);

	// print_statistics();
}

void Stats::print_statistics()
{
	printf("Statistics:\n");
	printf("-----------\n");
	printf("FTL Reads: %li\t Writes: %li\t Erases: %li\t Trims: %li\n", numFTLRead, numFTLWrite, numFTLErase,
		   numFTLTrim);
	printf("GC  Reads: %li\t Writes: %li\t Erases: %li\n", numGCRead, numGCWrite, numGCErase);
	printf("WL  Reads: %li\t Writes: %li\t Erases: %li\n", numWLRead, numWLWrite, numWLErase);
	printf("Log FTL Switch: %li Partial: %li Full: %li\n", numLogMergeSwitch, numLogMergePartial, numLogMergeFull);
	printf("Page FTL Convertions: %li\n", numPageBlockToPageConversion);
	printf("Cache Hits: %li Faults: %li Hit Ratio: %f\n", numCacheHits, numCacheFaults,
		   (double)numCacheHits / (double)(numCacheHits + numCacheFaults));
	printf("Memory Consumption:\n");
	printf("Tranlation: %li Cache: %li\n", numMemoryTranslation, numMemoryCache);
	printf("Reads: %li \tWrites: %li\n", numMemoryRead, numMemoryWrite);
	printf("-----------\n");
}

void Stats::write_csv(const std::string& fileName, const uint value, const uint begin)
{
	std::ofstream stream(fileName);
	stream << std::fixed;
	stream << std::setw(8) << begin << "," << std::setw(20) << std::setprecision(10) << value << std::endl;
}

void Stats::write_csv(const std::string& fileName, const ulong value, const uint begin)
{
	std::ofstream stream(fileName);
	stream << std::fixed;
	stream << std::setw(8) << begin << "," << std::setw(20) << std::setprecision(10) << value << std::endl;
}

void Stats::write_csv(const std::string& fileName, const double value, const uint begin)
{
	std::ofstream stream(fileName);
	stream << std::fixed;
	stream << std::setw(8) << begin << "," << std::setw(20) << std::setprecision(10) << value << std::endl;
}

void Stats::write_csv(const std::string& fileName, const std::vector<double>& vector, const uint begin)
{
	std::ofstream stream(fileName);
	stream << std::fixed;
	for (uint i = begin; i < begin + vector.size(); i++) {
		stream << std::setw(8) << i << "," << std::setw(20) << std::setprecision(10) << vector[i - begin] << std::endl;
	}
}

void Stats::write_csv(const std::string& fileName, const std::vector<uint>& vector, const uint begin)
{
	std::ofstream stream(fileName);
	for (uint i = begin; i < begin + vector.size(); i++) {
		stream << std::setw(8) << i << "," << std::setw(20) << vector[i - begin] << std::endl;
	}
}

void Stats::write_csv(const std::string& fileName, const std::vector<ulong>& vector, const uint begin)
{
	std::ofstream stream(fileName);
	for (uint i = begin; i < begin + vector.size(); i++) {
		stream << std::setw(8) << i << "," << std::setw(20) << vector[i - begin] << std::endl;
	}
}

std::string Stats::create_filename(const std::string& fileNameStart, const std::string& fieldName, const uint runID)
{
	std::stringstream sstr;
	sstr << fileNameStart;
	///@TODO Remove
	// if (not traceID.empty()) {
	//	sstr << "-" << traceID;
	//}
	sstr << "-" << fieldName;
	sstr << "." << std::setw(2) << runID << ".csv";
	return sstr.str();
}

void Stats::write_statistics_csv(const std::string& fileName, const uint runID, const Ssd& ssd)
{
	write_statistics_csv(fileName, runID);
	// Write PE cycles of all blocks
	const uint numBlocks = SSD_SIZE * PACKAGE_SIZE * DIE_SIZE * PLANE_SIZE;
	std::cout << numBlocks << " blocks on drive" << std::endl;
	std::vector<ulong> PE(numBlocks, 0);
	uint it = 0;
	for (uint package = 0; package < SSD_SIZE; package++) {
		for (uint die = 0; die < PACKAGE_SIZE; die++) {
			for (uint plane = 0; plane < DIE_SIZE; plane++) {
				for (uint block = 0; block < PLANE_SIZE; block++) {
					Block* block_ptr = ssd.get_block_pointer({package, die, plane, block, 0, address_valid::BLOCK});
					PE.at(it) = block_ptr->get_erase_count();
					it++;
				}
			}
		}
	}
	write_csv(create_filename(fileName, "PE", runID), PE, 0);
}

void Stats::write_statistics_csv(const std::string& fileName, const uint runID)
{
	std::cout << create_filename(fileName, "WA", runID) << std::endl;
	const double WA = getWA();
	write_csv(create_filename(fileName, "WA", runID), WA);
	std::cout << "WA: " << WA << std::endl;
	write_csv(create_filename(fileName, "fair", runID), PEfairness, 0);
	std::cout << "Last fairness: " << PEfairness.at(PEfairness.size() - 1) << std::endl;
	write_csv(create_filename(fileName, "end", runID), SSDendurance, 0);
	std::cout << "Last endurance: " << SSDendurance.at(SSDendurance.size() - 1) << std::endl;
	if (wl_supports_swap_info()) {
		write_csv(create_filename(fileName, "numWLswaps", runID), numWLSwaps, 0);
		std::cout << "#WL Swaps: " << nWLSwaps << std::endl;
		write_csv(create_filename(fileName, "swapcostWL", runID), WLSwapCost, 0);
	}
	if (ftl_supports_swap_info()) {
		write_csv(create_filename(fileName, "numFTLswaps", runID), numFTLSwaps, 0);
		std::cout << "#FTL Swaps: " << nFTLSwaps << std::endl;
		write_csv(create_filename(fileName, "swapcostFTL", runID), FTLSwapCost, 0);
	}
	/// FTL specific data
	// if (FTL_IMPLEMENTATION == IMPL_DWF) {
	//    std::cout << "DWF statistics" << std::endl;
	//    write_csv(create_filename(fileName, "WFEh", runID, traceID),
	//    WFEHotPagesDist);
	//    write_csv(create_filename(fileName, "WFIh", runID, traceID),
	//    WFIHotPagesDist);
	//} else
	if (FTL_IMPLEMENTATION == IMPL_HCWF) {
		write_csv(create_filename(fileName, "hotgcchain", runID), hotGCChainLength);
		write_csv(create_filename(fileName, "coldgcchain", runID), coldGCChainLength);
	}
	if (ftl_supports_hotcold_blocks()) {
		//    write_csv(create_filename(fileName, "hotpgf", runID, traceID),
		//    hotValidPages);
		//    write_csv(create_filename(fileName, "coldpgf", runID, traceID),
		//    coldValidPages);
		///@TODO Enable
		write_csv(create_filename(fileName, "hotblf", runID), hotBlocks);
		write_csv(create_filename(fileName, "coldblf", runID), coldBlocks);
		write_csv(create_filename(fileName, "hotvicdist", runID), hotVictim_ValidDist);
		write_csv(create_filename(fileName, "coldvicdist", runID), coldVictim_ValidDist);

		write_csv(create_filename(fileName, "hotvichotdist", runID), hotVictim_HotValidDist);
		write_csv(create_filename(fileName, "coldvichotdist", runID), coldVictim_HotValidDist);
		write_csv(create_filename(fileName, "numhotvic", runID), numHotVictimBlocks);
		write_csv(create_filename(fileName, "numcoldvic", runID), numColdVictimBlocks);
		//    write_csv(create_filename(fileName, "colderase", runID, traceID),
		//    firstColdErase);
	}
}

/*
void next_GC_invocation(const uint validPages, const uint hotValidPages, const
bool HWFReplaceOrWFE)
{
		if(FTL_IMPLEMENTATION == 6){
				// Hot valid pages of WFE or WFI on moment we replace them
				if(HWFReplaceOrWFE){
						WFEHotPagesDist[hotValidPages] = WFEHotPagesDist[hotValidPages] + 1;
				}else{
						WFIHotPagesDist[hotValidPages] = WFIHotPagesDist[hotValidPages] + 1;
				}
		//}else if(FTL_IMPLEMENTATION == 7){
		//    // Hot valid pages of victim!
		//    if(HWFReplaceOrWFE){
		//        HWFValidPages[validPages];
		//    }else{
		//        CWFValidPagesDist[validPages] = CWFValidPagesDist[validPages] + 1;
		//    }
		//}
}*/

void Stats::next_currentPE(const HotColdID* /*hcID*/)
{
	const uint numBlocks = SSD_SIZE * PACKAGE_SIZE * DIE_SIZE * PLANE_SIZE;
	currentPE--;
	PEfairness.push_back((double)(numGCErase + numWLErase) / (double)(numBlocks * (BLOCK_ERASES - currentPE)));
	SSDendurance.push_back((double)(numFTLWrite) / (double)(numBlocks * BLOCK_SIZE));

	if (ftl_supports_swap_info()) {
		numFTLSwaps.push_back(nFTLSwaps);
	}
	if (wl_supports_swap_info()) {
		numWLSwaps.push_back(nWLSwaps);
	}
	///@TODO Enable everything
	// if(hcID != nullptr)
	//{
	// hotValidPages.push_back(hcID->get_num_hot_pages());
	// coldValidPages.push_back(hcID->get_num_cold_pages());

	/// TODO Resolve this for HCWF....
	if (ftl_supports_hotcold_blocks()) {
		hotBlocks.push_back(numHotBlocks);
		coldBlocks.push_back(numColdBlocks);
		numHotVictimBlocks.push_back(numHotVictims);
		numColdVictimBlocks.push_back(numColdVictims);
	}
	//}
}

double Stats::getWA() const { return (double)(numGCWrite + numFTLWrite + numWLWrite) / (double)numFTLWrite; }

uint Stats::get_currentPE() const { return currentPE; }
