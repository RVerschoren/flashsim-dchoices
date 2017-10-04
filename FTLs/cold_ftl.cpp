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

void FtlImpl_COLD::initialize(const ulong numLPN)
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
            addr.package = RandNrGen::getInstance().get(SSD_SIZE);
            addr.die = RandNrGen::getInstance().get(PACKAGE_SIZE);
            addr.plane = RandNrGen::getInstance().get(DIE_SIZE);
            if(lpnIsHot)
            {
                addr.block =  RandNrGen::getInstance().get(maxHotBlocks);
            } else {
                addr.block =  maxHotBlocks + RandNrGen::getInstance().get(numColdBlocks);
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
    check_valid_pages(numLPN);
    #endif
    #ifndef NDEBUG
    check_block_hotness();
    check_ftl_hotness_integrity();
    #endif
}


void FtlImpl_COLD::initialize(const std::set<ulong> &uniqueLPNs)
{
    // Just assume for now that we have PAGE validity, we'll check it later anyway
    Address addr(0,0,0,0,0,PAGE);
    HWF = addr;
    HWFPtr = controller.get_block_pointer(HWF);

    ///@TODO  Review how to determine first cold block when not assuming 1 plane
    const double f = HOT_FRACTION;
    maxHotBlocks = std::ceil(f*PLANE_SIZE);
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
    ///@TODO Fix for(ulong it = 0; it < events.size(); it++)
    for(const ulong lpn : uniqueLPNs)
    {
        //const ulong lpn = events[it].get_logical_address();
        bool success = false;
        const bool lpnIsHot = hcID->is_hot(lpn);
        ///@TODO Fix const bool lpnIsHot = events[it].is_hot();
        if(map.find(lpn) == map.end())
        {
            while(not success)
            {
                addr.package = RandNrGen::getInstance().get(SSD_SIZE);
                addr.die = RandNrGen::getInstance().get(PACKAGE_SIZE);
                addr.plane = RandNrGen::getInstance().get(DIE_SIZE);
                if(lpnIsHot)
                {
                    addr.block = RandNrGen::getInstance().get(maxHotBlocks -1);//Correct for HWF
                } else {
                    addr.block =  maxHotBlocks + RandNrGen::getInstance().get(numColdBlocks);
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
    }
    #ifdef CHECK_VALID_PAGES
    check_valid_pages(map.size());
    #endif
    #ifndef NDEBUG
    check_block_hotness();
    check_ftl_hotness_integrity();
    #endif
}

FtlImpl_COLD::FtlImpl_COLD(Controller &controller, HotColdID *hcID, const uint d):
    FtlParent(controller), map(),  hcID(hcID), blockIsHot(SSD_SIZE,  std::vector<std::vector<std::vector<bool> > >(PACKAGE_SIZE, std::vector<std::vector<bool> >(DIE_SIZE, std::vector<bool>(PLANE_SIZE,false)))), d(d), FIFOCounter(0)
{
    return;
}

FtlImpl_COLD::~FtlImpl_COLD(void)
{
    return;
}

enum status FtlImpl_COLD::read(Event &event)
{
    controller.stats.numFTLRead++;
    const uint lpn = event.get_logical_address();
    if(map.find(lpn) != map.end()) event.set_address(map[lpn]);

    return SUCCESS;
}

void FtlImpl_COLD::gca_collect_COLD(Address &victimAddress, const bool replacingCWF)
{
    if(replacingCWF and d > 0) ///@TODO Remove FIFO hack
    {
        if(d == 0) ///@TODO Remove FIFO hack
        {
            FIFOCounter = (FIFOCounter + 1) % PLANE_SIZE;
            victimAddress.block = FIFOCounter;
        }else{
            Address address(victimAddress);
            assert(address.check_valid() >= PLANE);
            address.valid = BLOCK;//Make sure this is the case
            uint minValidPages = BLOCK_SIZE + 1;
            bool COLDvictim = false;
            for(uint i = 0; i < d; i++)
            {
                address.package = RandNrGen::getInstance().get(SSD_SIZE);
                address.die = RandNrGen::getInstance().get(PACKAGE_SIZE);
                address.plane = RandNrGen::getInstance().get(DIE_SIZE);
                address.block = RandNrGen::getInstance().get(PLANE_SIZE);
                assert(address.check_valid() >= BLOCK);

                const uint validPages = get_pages_valid(address);
                const bool potentialVictimIsColdBlock = not blockIsHot[address.package][address.die][address.plane][address.block];
                ///Only allow hot victim blocks if we don't have a cold victim already
                // This prefers cold victims and should limit the amount of hot->cold block conversions
                if(minValidPages > validPages
                        and
                        (
                            potentialVictimIsColdBlock
                            or not COLDvictim
                        )
                    ){
                    minValidPages = validPages;
                    victimAddress = address;
                    COLDvictim = potentialVictimIsColdBlock;
                }
            }
        }
        victimAddress.valid = PAGE;
    }else{
        garbage->collect(victimAddress);
    }
}

enum status FtlImpl_COLD::write(Event &event)
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
    if(map.find(lpn) != map.end()) get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);

    const bool lpnIsHot = hcID->is_hot(lpn);

    while( (lpnIsHot and HWFPtr->get_next_page(HWF) != SUCCESS)
           or (not lpnIsHot and CWFPtr->get_next_page(CWF) != SUCCESS) )//Still space in WF
    {
        const bool HWFInitiated = HWFPtr->get_next_page(HWF) != SUCCESS; // HWF initiated cleaning cycle

        assert(not HWFInitiated or HWFPtr->get_pages_empty() == 0);
        assert(HWFInitiated or CWFPtr->get_pages_empty() == 0);

        //Need to select a victim block through GC
        Address  victim = (map.find(lpn) != map.end())?  map[lpn] : HWF;
        Block *victimPtr;
        do {
            this->gca_collect_COLD(victim, not HWFInitiated);
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


enum status FtlImpl_COLD::trim(Event &event)
{
    ///@TODO Implement
    const uint lpn = event.get_logical_address();
    event.set_address(map[lpn]);
    //get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);//Should we do this here?
    return SUCCESS;
}



void FtlImpl_COLD::check_valid_pages(const ulong numLPN)
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
                            //std::cout << "\t" << p << " : " << controller.get_page_pointer(addr)->get_logical_address() << std::endl;
                            numPages++;
                        }
                    }
                }
            }
        }
    }
    assert(numPages == numLPN);
}

void FtlImpl_COLD::check_block_hotness()
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

void FtlImpl_COLD::check_ftl_hotness_integrity()
{
    for(std::pair<ulong,Address> pair : map)
    {
        const ulong lpn = pair.first;
        const Address &addr = pair.second;
        assert(blockIsHot[addr.package][addr.die][addr.plane][addr.block] == hcID->is_hot(lpn));
    }
}
