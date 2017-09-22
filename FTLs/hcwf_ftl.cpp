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

void FtlImpl_HCWF::initialize()
{
    // Just assume for now that we have PAGE validity, we'll check it later anyway
    Address addr(0,0,0,0,0,PAGE);
    HWF = addr;
    HWFPtr = controller.get_block_pointer(HWF);

    ///@TODO  Review how to determine first cold block when not assuming 1 plane
    const double f = HOT_FRACTION;
    const uint maxHotBlocks = std::ceil(f*PLANE_SIZE) + 1;
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
    assert(numHotBlocks <= maxHotBlocks);

    /// Initialize at random
    for(unsigned int lpn = 0; lpn < map.size(); lpn++)
    {
        bool success = false;
        const bool lpnIsHot = hcID.is_hot(lpn);
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

}


FtlImpl_HCWF::FtlImpl_HCWF(Controller &controller):
    FtlParent(controller), numLPN((1.0-SPARE_FACTOR)*BLOCK_SIZE*PLANE_SIZE*DIE_SIZE*PACKAGE_SIZE), map(), hcID(numLPN,HOT_FRACTION)
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

enum status FtlImpl_HCWF::erase_victim(Event &event, Address & victim, std::vector<ulong> &validLPNs){
    #ifndef NDEBUG
    std::cout << "ERASE" << std::endl;
    #endif
    double startTime = event.get_start_time();
    for(uint p = 0; p < BLOCK_SIZE; p++){
        //Pretend to read the data to copy it
        Event readEvent(READ, event.get_logical_address(), 1, startTime);
        victim.page = p;
        victim.valid = PAGE;
        readEvent.set_address(victim);

        if(controller.issue(readEvent) == SUCCESS){//Page is valid?
            event.incr_time_taken(readEvent.get_time_taken());
            startTime += readEvent.get_time_taken();
            const ulong lpn = controller.get_page_pointer(victim)->get_logical_address();
            validLPNs.push_back(lpn);//Sneakily get the LPN behind the controller's back
        }
    }
    /// Erase the victim block
    Event eraseEvent(ERASE, event.get_logical_address(), 1, event.get_start_time());
    eraseEvent.set_address(victim);

    enum status eraseStatus = controller.issue(eraseEvent);
    if(eraseStatus == FAILURE){
        printf("Erase failed");
    }
    event.incr_time_taken(eraseEvent.get_time_taken());

    return eraseStatus;
}


enum status FtlImpl_HCWF::copy_to_block(Event &event, double startTime, const std::vector<ulong> &validLPNs, Address &block, const Block *blockPtr)
{
    return copy_to_blocks(event, startTime, validLPNs, validLPNs.size(), block, blockPtr, block, blockPtr);
}

enum status FtlImpl_HCWF::copy_to_blocks(Event &event, double startTime, const std::vector<ulong> &validLPNs, uint freeInBlock1,
                                                                                    Address &block1, const Block *block1Ptr, Address &block2, const Block *block2Ptr)
{
    #ifndef NDEBUG
    std::cout << "COPY" << std::endl;
    #endif
    /// Copy everything back!
    for(uint p = 0; p < freeInBlock1; p++){
        //Pretend to read the data to copy it
        Event copyEvent(WRITE, validLPNs[p], 1, startTime);
        if(block1Ptr->get_next_page(block1) == SUCCESS){
            assert(controller.get_page_pointer(block1)->get_state() == EMPTY);
            copyEvent.set_address(block1);

            assert(block1.block == copyEvent.get_address().block);
            assert(block1.page == copyEvent.get_address().page);
            if(controller.issue(copyEvent) == SUCCESS){
                event.incr_time_taken(copyEvent.get_time_taken());
                startTime += copyEvent.get_time_taken();
                map[validLPNs[p]] = block1;
            }else{
                assert(false);
                return FAILURE;
            }
        }else{
            assert(false);
            return FAILURE;
        }
    }
    for(uint p = freeInBlock1; p < validLPNs.size() ; p++){
        //Pretend to read the data to copy it
        Event copyEvent(WRITE, validLPNs[p], 1, startTime);
        if(block2Ptr->get_next_page(block2) == SUCCESS){
            assert(controller.get_page_pointer(block2)->get_state() == EMPTY);
            copyEvent.set_address(block2);

            assert(block2.block == copyEvent.get_address().block);
            assert(block2.page == copyEvent.get_address().page);
            if(controller.issue(copyEvent) == SUCCESS){
                event.incr_time_taken(copyEvent.get_time_taken());
                startTime += copyEvent.get_time_taken();
                map[validLPNs[p]] = block2;
            }else{
                assert(false);
                return FAILURE;
            }
        }else{
            assert(false);
            return FAILURE;
        }
    }
    return SUCCESS;
}

enum status FtlImpl_HCWF::write(Event &event)
{
    #ifndef NDEBUG
        uint numPages = 0;
        std::cout << "NUMPAGES " << event.get_logical_address() << std::endl;
        for(uint package = 0; package < SSD_SIZE; package++){
            for(uint die = 0; die < PACKAGE_SIZE; die++){
                for(uint plane = 0; plane < DIE_SIZE; plane++){
                    for(uint b = 0; b < PLANE_SIZE; b++){
                        std::cout << "    BLOCK" << b << std::endl;
                        for(uint p = 0; p < BLOCK_SIZE; p++){
                            Address addr(package,die,plane,b,p,PAGE);
                            if(controller.get_page_pointer(addr)->get_state() == VALID){
                                std::cout << "\t" << p << " : " << controller.get_page_pointer(addr)->get_logical_address() << std::endl;
                                numPages++;
                            }
                        }
                    }
                }
            }
        }
        assert(numPages == numLPN);
    #endif
    controller.stats.numFTLWrite++;
    const ulong lpn = event.get_logical_address();

    ///Invalidate previous page
    get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);

    const bool lpnIsHot = hcID.is_hot(lpn);

    while(HWFPtr->get_next_page(HWF) != SUCCESS or CWFPtr->get_next_page(CWF) != SUCCESS)//Still space in WF
    {
        const bool HWFInitiated = HWFPtr->get_next_page(HWF) != SUCCESS; // HWF initiated cleaning cycle
        //Need to select a victim block through GC
        Address  victim = map[lpn];
        Block *victimPtr;
        do {
            garbage->collect(victim);
            victimPtr = controller.get_block_pointer(victim);
        } while(victimPtr != HWFPtr and victimPtr != CWFPtr);
        const bool victimIsHot = blockIsHot[victim.package][victim.die][victim.plane][victim.block];

        const uint j = victimPtr->get_pages_valid();
        assert(j <= BLOCK_SIZE);
        const uint k = HWFInitiated? CWFPtr->get_pages_empty() : HWFPtr->get_pages_empty();
        assert(k <= BLOCK_SIZE);

        std::vector<ulong> victimValidLPNs;
        enum status eraseStatus = erase_victim(event,  victim, victimValidLPNs);

        controller.stats.numGCRead += victimValidLPNs.size();
        controller.stats.numGCErase++;
        controller.stats.victimValidDist[victimValidLPNs.size()] = controller.stats.victimValidDist[victimValidLPNs.size()] + 1;


        if(HWFInitiated)
        {
            if(victimIsHot)
            {
                enum status copyStatus = copy_to_block(event, event.get_start_time()+event.get_time_taken(), victimValidLPNs,
                                                                                           victim, victimPtr);
            }else if(j <= k){ // Sufficient space to copy everything to otherWF
                enum status copyStatus = copy_to_block(event, event.get_start_time()+event.get_time_taken(), victimValidLPNs,
                                                                                               CWF,CWFPtr);
                /// Victim replaces the correct WF
                HWF = victim;
                HWFPtr =victimPtr;
            } else {
                enum status copyStatus = copy_to_blocks(event, event.get_start_time()+event.get_time_taken(), victimValidLPNs,k,
                                                                                               CWF, CWFPtr, victim, victimPtr);
                /// Victim replaces the other WF
                CWF = victim;
                CWFPtr =victimPtr;
                blockIsHot[victim.package][victim.die][victim.plane][victim.block] = false;
                numHotBlocks--;
            }

        }else{ // CWF was full
            if(not victimIsHot)
            {
                enum status copyStatus = copy_to_block(event, event.get_start_time()+event.get_time_taken(), victimValidLPNs,
                                                                                           victim, victimPtr);
            }else if(j <= k){ // Sufficient space to copy everything to otherWF
                enum status copyStatus = copy_to_block(event, event.get_start_time()+event.get_time_taken(), victimValidLPNs,
                                                                                               HWF,HWFPtr);
                /// Victim replaces the correct WF
                CWF = victim;
                CWFPtr =victimPtr;
            } else {
                enum status copyStatus = copy_to_blocks(event, event.get_start_time()+event.get_time_taken(), victimValidLPNs,k,
                                                                                               HWF, HWFPtr, victim, victimPtr);
                /// Victim replaces the other WF
                HWF = victim;
                HWFPtr =victimPtr;
                blockIsHot[victim.package][victim.die][victim.plane][victim.block] = true;
                numHotBlocks++;
            }
        }

        controller.stats.numGCWrite += victimValidLPNs.size();
        const ulong numErasesOfVictim = BLOCK_ERASES - victimPtr->get_erases_remaining();
        if(controller.stats.get_currentPE() < numErasesOfVictim){
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
