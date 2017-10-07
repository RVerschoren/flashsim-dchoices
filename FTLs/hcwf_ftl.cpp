/* Copyright 2017 Robin Verschoren */

/* hcwf_ftl.cpp  */

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

/* Implements a very simple page-level FTL without merge */

#include <new>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "../ssd.h"
#include "../util.h"

using namespace ssd;

ulong block_address_to_linear_address(const Address &address)
{
    return address.block + PLANE_SIZE*(address.plane + DIE_SIZE*(address.die + PACKAGE_SIZE*address.package));
}

void FtlImpl_HCWF::initialize(const ulong numLPN)
{
    // Just assume for now that we have PAGE validity, we'll check it later anyway
    Address addr(0,0,0,0,0,PAGE);
    HWF = addr;
    HWFPtr = controller.get_block_pointer(HWF);

    ///@TODO  Review how to determine first cold block when not assuming 1 plane
    const double f = HOT_FRACTION;
    maxHotBlocks = std::ceil(f*PLANE_SIZE) + 1;
    const uint numColdBlocks = PLANE_SIZE - maxHotBlocks;
    CWF = Address(0,0,0,maxHotBlocks,0,PAGE);
    CWFPtr = controller.get_block_pointer(CWF);

    numHotBlocks = 0;
    /// Set hotness
    for(uint package = 0; package < SSD_SIZE; package++){
        for(uint die = 0; die < PACKAGE_SIZE; die++){
            for(uint plane= 0; plane < DIE_SIZE; plane++){
                for(uint block = 0; block < PLANE_SIZE; block++){
                    blockIsHot[package][die][plane][block] = block < maxHotBlocks;
                    if(block < maxHotBlocks) numHotBlocks++;
                }
            }
        }
    }
    blockIsHot[HWF.package][HWF.die][HWF.plane][HWF.block] = true;
    blockIsHot[CWF.package][CWF.die][CWF.plane][CWF.block] = false;
    assert(numHotBlocks <= maxHotBlocks);
    assert(numHotBlocks <= PLANE_SIZE);

    /// Initialize at random
    for(unsigned int lpn = 0; lpn < numLPN; lpn++)
    {
        bool success = false;
        const bool lpnIsHot = hcID->is_hot(lpn);
        while(not success)
        {
            #ifndef SINGLE_PLANE
            addr.package = RandNrGen::get(SSD_SIZE);
            addr.die = RandNrGen::get(PACKAGE_SIZE);
            #endif
            addr.plane = RandNrGen::get(DIE_SIZE);
            if(lpnIsHot)
            {
                addr.block =  RandNrGen::get(maxHotBlocks);
            } else {
                addr.block =  maxHotBlocks + RandNrGen::get(numColdBlocks);
            }
            assert(addr.check_valid() >= BLOCK);

            Block * block = controller.get_block_pointer(addr);

            if(block != HWFPtr and block != CWFPtr and block->get_next_page(addr) == SUCCESS){
                ///@TODO Should we avoid the controller? Perhaps, to not count the time taken for initializing the disk...
                Event evt(WRITE, lpn, 1, 0);
                evt.set_address(addr);
                block->write(evt);
                map.push_back(addr);
                success = true;
            }
        }
    }
    #ifdef CHECK_VALID_PAGES
    check_valid_pages(numLPN);
    #endif
    #ifndef NDEBUG
    check_block_hotness();
    check_ftl_hotness_integrity();
    #endif
}

/*
void FtlImpl_HCWF::initialize(const std::set<ulong> &uniqueLPNs)
{
    // Just assume for now that we have PAGE validity, we'll check it later anyway
    Address addr(0,0,0,0,0,PAGE);
    HWF = addr;
    HWFPtr = controller.get_block_pointer(HWF);

    ///@TODO  Review how to determine first cold block when not assuming 1 plane
    const double f = HOT_FRACTION;
    maxHotBlocks = std::ceil(f*PLANE_SIZE) + 1;
    const uint numColdBlocks = PLANE_SIZE - maxHotBlocks;
    CWF = Address(0,0,0,maxHotBlocks,0,PAGE);
    CWFPtr = controller.get_block_pointer(CWF);

    numHotBlocks = 0;
    /// Set hotness
    for(uint package = 0; package < SSD_SIZE; package++){
        for(uint die = 0; die < PACKAGE_SIZE; die++){
            for(uint plane= 0; plane < DIE_SIZE; plane++){
                for(uint block = 0; block < PLANE_SIZE; block++){
                    blockIsHot[package][die][plane][block] = block < maxHotBlocks;
                    if(block < maxHotBlocks) numHotBlocks++;
                }
            }
        }
    }
    blockIsHot[HWF.package][HWF.die][HWF.plane][HWF.block] = true;
    blockIsHot[CWF.package][CWF.die][CWF.plane][CWF.block] = false;
    assert(numHotBlocks <= maxHotBlocks);
    assert(numHotBlocks <= PLANE_SIZE);

    /// Initialize with events
    for(const ulong lpn : uniqueLPNs)
    {
        //const ulong lpn = events[it].get_logical_address();
        bool success = false;
        const bool lpnIsHot = hcID->is_hot(lpn);
        ///@TODO Fix const bool lpnIsHot = events[it].is_hot();

         while(not success)
         {
            #ifndef SINGLE_PLANE
             addr.package = RandNrGen::get(SSD_SIZE);
             addr.die = RandNrGen::.get(PACKAGE_SIZE);
            #endif
             addr.plane = RandNrGen::get(DIE_SIZE);
             if(lpnIsHot)
             {
                 addr.block =  RandNrGen::get(maxHotBlocks);
             } else {
                 addr.block =  maxHotBlocks + RandNrGen::get(numColdBlocks);
             }
             assert(addr.check_valid() >= BLOCK);

             Block * block = controller.get_block_pointer(addr);

             if(block != HWFPtr and block != CWFPtr and block->get_next_page(addr) == SUCCESS){
                 ///@TODO Should we avoid the controller? Perhaps, to not count the time taken for initializing the disk...
                 Event evt(WRITE, lpn, 1, 0);
                 evt.set_address(addr);
                 block->write(evt);
                 map[lpn] = addr;
                 success = true;
             }
         }
     }
    #ifdef CHECK_VALID_PAGES
    check_valid_pages(map.size());
    #endif
    #ifndef NDEBUG
    check_block_hotness();
    check_ftl_hotness_integrity();
    #endif
}
*/

FtlImpl_HCWF::FtlImpl_HCWF(Controller &controller, HotColdID *hcID):
    FtlParent(controller), map(),  hcID(hcID), blockIsHot(SSD_SIZE,  std::vector<std::vector<std::vector<bool> > >(PACKAGE_SIZE, std::vector<std::vector<bool> >(DIE_SIZE, std::vector<bool>(PLANE_SIZE,false))))
{
    return;
}

FtlImpl_HCWF::~FtlImpl_HCWF(void)
{
    return;
}

enum status FtlImpl_HCWF::read(Event &event)
{
    controller.stats.numFTLRead++;
    const uint lpn = event.get_logical_address();
    event.set_address(map[lpn]);

    return SUCCESS;
}

enum status FtlImpl_HCWF::write(Event &event)
{
    #if defined DEBUG || defined CHECK_VALID_PAGES
    check_valid_pages(map.size());
    #endif
    #ifndef NDEBUG
    check_block_hotness();
    check_ftl_hotness_integrity();
    #endif
    controller.stats.numFTLWrite++;
    const ulong lpn = event.get_logical_address();

    ///Invalidate previous page
    get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);

    const bool lpnIsHot = hcID->is_hot(lpn);

    while( (lpnIsHot and HWFPtr->get_next_page(HWF) != SUCCESS)
           or (not lpnIsHot and CWFPtr->get_next_page(CWF) != SUCCESS) )//Still space in WF
    {
        const bool HWFInitiated = HWFPtr->get_next_page(HWF) != SUCCESS; // HWF initiated cleaning cycle

        assert(not HWFInitiated or HWFPtr->get_pages_empty() == 0);
        assert(HWFInitiated or CWFPtr->get_pages_empty() == 0);

        //Need to select a victim block through GC
        Address  victim = map[lpn];
        Block *victimPtr;
        do {
            garbage->collect(victim);
            victimPtr = controller.get_block_pointer(victim);
        } while(victimPtr == HWFPtr or victimPtr == CWFPtr);

        const bool victimIsHot = blockIsHot[victim.package][victim.die][victim.plane][victim.block];

        const uint j = victimPtr->get_pages_valid();
        assert(j <= BLOCK_SIZE);
        //const uint k = HWFInitiated? CWFPtr->get_pages_empty() : HWFPtr->get_pages_empty();
        //assert(k <= BLOCK_SIZE);

        controller.stats.numGCRead += j;
        controller.stats.numGCErase += 1;
        controller.stats.victimValidDist[j] = controller.stats.victimValidDist[j] + 1;
        //std::cout << "\t moving " << j << " pages"<<std::endl;
        if(HWFInitiated)
        {
            if(victimIsHot)
            {
                victimPtr->_erase_and_copy(event, HWF, HWFPtr, //Copy to self
                                           [this,&victim](const ulong lpn, const Address &addr) {
                                                map[lpn] = addr;
                                            },
                                           [this](const ulong lpn, const uint newPage) {
                                                map[lpn].page = newPage;
                                            }
                );
                /// Victim replaces the correct WF
                HWF = victim;
                HWFPtr =victimPtr;
            }else if(j <= CWFPtr->get_pages_empty()){ // Sufficient space to copy everything to CWF
                victimPtr->_erase_and_copy(event, CWF, CWFPtr, //Copy to CWF where possible
                                           [this,&victim](const ulong lpn, const Address &addr) {
                                                map[lpn] = addr;
                                            },
                                           [this](const ulong lpn, const uint newPage) {
                                                map[lpn].page = newPage;
                                            }
                );
                /// Victim replaces the correct WF
                HWF = victim;
                HWFPtr =victimPtr;
                blockIsHot[victim.package][victim.die][victim.plane][victim.block] = true;
                numHotBlocks++;
            } else {
                victimPtr->_erase_and_copy(event, CWF, CWFPtr, //Copy to CWF where possible
                                           [this,&victim](const ulong lpn, const Address &addr) {
                                                map[lpn] = addr;
                                            },
                                           [this](const ulong lpn, const uint newPage) {
                                                map[lpn].page = newPage;
                                            }
                );
                /// Victim replaces the other WF
                CWF = victim;
                CWFPtr =victimPtr;
            }

        }else{ // CWF was full
            if(not victimIsHot)
            {
                victimPtr->_erase_and_copy(event, CWF, CWFPtr, //Copy to self
                                           [this,&victim](const ulong lpn, const Address &addr) {
                                                map[lpn] = addr;
                                            },
                                           [this](const ulong lpn, const uint newPage) {
                                                map[lpn].page = newPage;
                                            }
                );
                /// Victim replaces the correct WF
                CWF = victim;
                CWFPtr =victimPtr;
            }else if(j <= HWFPtr->get_pages_empty()){ // Sufficient space to copy everything to HWF
                victimPtr->_erase_and_copy(event, HWF, HWFPtr, //Copy to HWF where possible
                                           [this,&victim](const ulong lpn, const Address &addr) {
                                                map[lpn] = addr;
                                            },
                                           [this](const ulong lpn, const uint newPage) {
                                                map[lpn].page = newPage;
                                            }
                );
                /// Victim replaces the correct WF
                CWF = victim;
                CWFPtr =victimPtr;
                blockIsHot[victim.package][victim.die][victim.plane][victim.block] = false;
                numHotBlocks--;
            } else {
                victimPtr->_erase_and_copy(event, HWF, HWFPtr, //Copy to HWF where possible
                                           [this,&victim](const ulong lpn, const Address &addr) {
                                                map[lpn] = addr;
                                            },
                                           [this](const ulong lpn, const uint newPage) {
                                                map[lpn].page = newPage;
                                            }
                );
                /// Victim replaces the other WF
                HWF = victim;
                HWFPtr =victimPtr;
            }
        }
        controller.stats.numGCWrite += j;
        if(controller.stats.get_currentPE() >= victimPtr->get_erases_remaining()){
            controller.stats.next_currentPE();
        }

    }
    if(lpnIsHot)
    {
        event.set_address(HWF);//Tell WF to write to this (next) page
        map[lpn] = HWF;
    }else{
        event.set_address(CWF);//Tell WF to write to this (next) page
        map[lpn] = CWF;
    }

    #if defined DEBUG || defined CHECK_VALID_PAGES
    check_valid_pages(map.size());
    #endif

    return SUCCESS;
}


enum status FtlImpl_HCWF::trim(Event &event)
{
    ///@TODO Implement
    const uint lpn = event.get_logical_address();
    event.set_address(map[lpn]);
    //get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);//Should we do this here?
    return SUCCESS;
}



void FtlImpl_HCWF::check_valid_pages(const ulong numLPN)
{
    uint numPages = 0;
    for(uint package = 0; package < SSD_SIZE; package++){
        for(uint die = 0; die < PACKAGE_SIZE; die++){
            for(uint plane = 0; plane < DIE_SIZE; plane++){
                for(uint b = 0; b < PLANE_SIZE; b++){
                    //std::cout << "    BLOCK" << b << std::endl;
                    for(uint p = 0; p < BLOCK_SIZE; p++){
                        Address addr(package,die,plane,b,p,PAGE);
                        if(controller.get_page_pointer(addr)->get_state() == VALID){
                            numPages++;
                        }
                    }
                }
            }
        }
    }
    assert(numPages == numLPN);
}

void FtlImpl_HCWF::check_block_hotness()
{
    uint numBlocks= 0;
    for(uint package = 0; package < SSD_SIZE; package++){
        for(uint die = 0; die < PACKAGE_SIZE; die++){
            for(uint plane = 0; plane < DIE_SIZE; plane++){
                for(uint b = 0; b < PLANE_SIZE; b++){
                        if(blockIsHot[package][die][plane][b]) numBlocks++;
                }
            }
        }
    }
    assert(numBlocks == numHotBlocks);
}

void FtlImpl_HCWF::check_ftl_hotness_integrity()
{
    for(uint it = 0; it < map.size(); it++)
    {
        const ulong lpn = it;
        const Address &addr = map[it];
        assert(blockIsHot[addr.package][addr.die][addr.plane][addr.block] == hcID->is_hot(lpn));
    }
}
