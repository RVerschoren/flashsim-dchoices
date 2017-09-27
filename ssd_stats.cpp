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

#include <new>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "ssd.h"

using namespace ssd;

Stats::Stats()
{
    reset();
}

void Stats::reset()
{
    // FTL
    numFTLRead = 0;
    numFTLWrite = 0;
    numFTLErase = 0;
    numFTLTrim = 0;

    //GC
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
    PEfairness.clear();
    PEfairness.push_back(0.0); // Initial data point
    SSDendurance.clear();
    SSDendurance.push_back(0.0); // Initial data point
    PEDistribution.clear();

    // DWF
    WFEHotPagesDist.clear();
    for(uint i = 0; i < BLOCK_SIZE; i++){
        WFEHotPagesDist.push_back(0);
    }
    WFIHotPagesDist.clear();
    for(uint i = 0; i < BLOCK_SIZE; i++){
        WFIHotPagesDist.push_back(0);
    }
    victimValidDist.clear();
    for(uint i = 0; i < BLOCK_SIZE; i++){
        victimValidDist.push_back(0);
    }

    // HCWF
    hotValidPages.clear();
    coldValidPages.clear();
    hotBlocks.clear();
    coldBlocks.clear();

    // Memory consumptions (Bytes)
    numMemoryTranslation = 0;
    numMemoryCache = 0;

    numMemoryRead = 0;
    numMemoryWrite = 0;
}

void Stats::reset_statistics()
{
    reset();
}

void Stats::write_header(FILE *stream)
{
    fprintf(stream, "numFTLRead;numFTLWrite;numFTLErase;numFTLTrim;numGCRead;numGCWrite;numGCErase;numWLRead;numWLWrite;numWLErase;numLogMergeSwitch;numLogMergePartial;numLogMergeFull;numPageBlockToPageConversion;numCacheHits;numCacheFaults;numMemoryTranslation;numMemoryCache;numMemoryRead;numMemoryWrite\n");
}

void Stats::write_statistics(FILE *stream)
{
    fprintf(stream, "%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;%li;\n",
            numFTLRead, numFTLWrite, numFTLErase, numFTLTrim,
            numGCRead, numGCWrite, numGCErase,
            numWLRead, numWLWrite, numWLErase,
            numLogMergeSwitch, numLogMergePartial, numLogMergeFull,
            numPageBlockToPageConversion,
            numCacheHits, numCacheFaults,
            numMemoryTranslation,
            numMemoryCache,
            numMemoryRead,numMemoryWrite);

    //print_statistics();
}

void Stats::print_statistics()
{
    printf("Statistics:\n");
    printf("-----------\n");
    printf("FTL Reads: %li\t Writes: %li\t Erases: %li\t Trims: %li\n", numFTLRead, numFTLWrite, numFTLErase, numFTLTrim);
    printf("GC  Reads: %li\t Writes: %li\t Erases: %li\n", numGCRead, numGCWrite, numGCErase);
    printf("WL  Reads: %li\t Writes: %li\t Erases: %li\n", numWLRead, numWLWrite, numWLErase);
    printf("Log FTL Switch: %li Partial: %li Full: %li\n", numLogMergeSwitch, numLogMergePartial, numLogMergeFull);
    printf("Page FTL Convertions: %li\n", numPageBlockToPageConversion);
    printf("Cache Hits: %li Faults: %li Hit Ratio: %f\n", numCacheHits, numCacheFaults, (double)numCacheHits/(double)(numCacheHits+numCacheFaults));
    printf("Memory Consumption:\n");
    printf("Tranlation: %li Cache: %li\n", numMemoryTranslation, numMemoryCache);
    printf("Reads: %li \tWrites: %li\n", numMemoryRead, numMemoryWrite);
    printf("-----------\n");
}

void Stats::write_csv(const std::string fileName, const double value, const uint begin)
{
     std::ofstream stream(fileName);
    stream << std::fixed;
    stream << std::setw(8) << begin << "," << std::setw(20) << std::setprecision(10) << value << std::endl;
}

void Stats::write_csv(const std::string fileName, const std::vector<double> &vector, const uint begin)
{
     std::ofstream stream(fileName);
    stream << std::fixed;
    for(uint i = begin; i < begin+vector.size(); i++)
    {
        stream << std::setw(8) << i << "," << std::setw(20) << std::setprecision(10) << vector[i-begin] << std::endl;
    }
}

void Stats::write_csv(const std::string fileName, const std::vector<uint> &vector, const uint begin)
{
     std::ofstream stream(fileName);
    for(uint i = begin; i < begin+vector.size(); i++)
    {
        stream << std::setw(8) << i << "," << std::setw(20) << vector[i-begin] << std::endl;
    }
}

void Stats::write_csv(const std::string fileName, const std::vector<ulong> &vector,const  uint begin)
{
    std::ofstream stream(fileName);
    for(uint i = begin; i < begin+vector.size(); i++)
    {
        stream << std::setw(8) << i << "," << std::setw(20) << vector[i-begin] << std::endl;
    }
}

std::string Stats::create_filename(const std::string fileNameStart, const std::string fieldName, const uint runID, const std::string traceID)
{
    std::stringstream sstr;
    sstr << fileNameStart;
    if(not traceID.empty())
    {
        sstr << "-" << traceID;
    }
    sstr << "-" << fieldName;
    sstr << "." << std::setw(2) << runID << ".csv";
    return sstr.str();
}

void Stats::write_statistics_csv(const std::string fileName, const uint runID, const std::string traceID)
{
    write_csv(create_filename(fileName, "WA", runID, traceID), (double) (numGCWrite+numFTLWrite)/(double) numFTLWrite);
    write_csv(create_filename(fileName, "fair", runID, traceID), PEfairness, 0);
    write_csv(create_filename(fileName, "end", runID, traceID), SSDendurance, 0);
   /// FTL specific data
    if(FTL_IMPLEMENTATION == 6){ // DWF
        std::cout << "DWF statistics" << std::endl;
        write_csv(create_filename(fileName, "WFEh", runID, traceID), WFEHotPagesDist);
        write_csv(create_filename(fileName, "WFIh", runID, traceID), WFIHotPagesDist);
    }else if(FTL_IMPLEMENTATION == 7){ // HCWF
        write_csv(create_filename(fileName, "hotpgf", runID, traceID), hotValidPages);
        write_csv(create_filename(fileName, "coldpgf", runID, traceID), coldValidPages);
        write_csv(create_filename(fileName, "hotblf", runID, traceID), hotBlocks);
        write_csv(create_filename(fileName, "coldblf", runID, traceID), coldBlocks);
    }

}
/*
void next_GC_invocation(const uint validPages, const uint hotValidPages, const bool HWFReplaceOrWFE)
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

void Stats::next_currentPE(const HotColdID *hcID)
{
    currentPE--;
    PEfairness.push_back((double) numGCErase / (double)(PLANE_SIZE*(BLOCK_ERASES - currentPE)));
    SSDendurance.push_back((double) numFTLWrite/ (double)(PLANE_SIZE*BLOCK_SIZE));
    //if(hcID != nullptr)
    //{
    //    hotValidPages.push_back(hcID->get_num_hot_pages());
    //    coldValidPages.push_back(hcID->get_num_cold_pages());
    //
    //    ///@TODO Resolve this for HCWF....
    //    //if(FTL_IMPLEMENTATION == 7){
    //    //    hotBlocks.push_back(hcID->get_num_hot_blocks());
    //    //    coldBlocks.push_back(hcID->get_num_cold_blocks());
    //    //}
    //}
}

uint Stats::get_currentPE() const
{
    return currentPE;
}
