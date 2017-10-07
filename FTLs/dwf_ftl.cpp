/* Copyright 2017 Robin Verschoren */

/* dwf_ftl.cpp  */

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

void FtlImpl_DWF::initialize(const ulong numLPN)
{
    // Just assume for now that we have PAGE validity, we'll check it later anyway
    Address addr(0,0,0,0,0,PAGE);
    WFE = addr;
    WFEPtr = controller.get_block_pointer(WFE);
    WFI = Address(0,0,0,1,0,PAGE);
    WFIPtr = controller.get_block_pointer(WFI);

    for(ulong lpn = 0; lpn < numLPN; lpn++)
    {
        bool success = false;
        //const bool lpnIsHot = hcID.is_hot(lpn);
        while(not success)
        {
            #ifndef SINGLE_PLANE
            addr.package = RandNrGen::get(SSD_SIZE);
            addr.die = RandNrGen::get(PACKAGE_SIZE);
            #endif
            addr.plane = RandNrGen::get(DIE_SIZE);
            addr.block = RandNrGen::get(PLANE_SIZE);
            assert(addr.check_valid() >= BLOCK);

            Block * block = controller.get_block_pointer(addr);

            if(block != WFEPtr and block != WFIPtr and block->get_next_page(addr) == SUCCESS){
                ///@TODO Should we avoid the controller? Perhaps, to not count the time taken for initializing the disk...
                Event evt(WRITE, lpn, 1, 0);
                evt.set_address(addr);
                block->write(evt);
                map.push_back(addr);
                if(hcID->is_hot(lpn))
                {
                    hotValidPages[addr.package][addr.die][addr.plane][addr.block]++;
                }
                success = true;
            }
        }
    }
}

/*void FtlImpl_DWF::initialize(const std::set<ulong> &uniqueLPNs)
{
    #if defined DEBUG || defined CHECK_VALID_PAGES
    check_valid_pages(map.size());
    check_ftl_integrity();
    #endif

    // Just assume for now that we have PAGE validity, we'll check it later anyway
    Address addr(0,0,0,0,0,PAGE);
    WFE = addr;
    WFEPtr = controller.get_block_pointer(WFE);
    WFI = Address(0,0,0,1,0,PAGE);
    WFIPtr = controller.get_block_pointer(WFI);

    //const uint maxHotBlocks = std::ceil(HOT_FRACTION*PLANE_SIZE) + 1;
    //const uint numColdBlocks = PLANE_SIZE - maxHotBlocks;

    for(const ulong lpn : uniqueLPNs)
    {
        bool success = false;
        if(map.find(lpn) == map.end())
        {
            while(not success)
            {
                addr.package = RandNrGen::getInstance().get(SSD_SIZE);
                addr.die = RandNrGen::getInstance().get(PACKAGE_SIZE);
                addr.plane = RandNrGen::getInstance().get(DIE_SIZE);
                addr.block = RandNrGen::getInstance().get(PLANE_SIZE);
                assert(addr.check_valid() >= BLOCK);

                Block * block = controller.get_block_pointer(addr);

                if(block != WFEPtr and block != WFIPtr and block->get_next_page(addr) == SUCCESS){
                    #if defined DEBUG || defined CHECK_VALID_PAGES
                    check_valid_pages(map.size());
                    check_ftl_integrity();
                    #endif
                    ///@TODO Should we avoid the controller? Perhaps, to not count the time taken for initializing the disk...
                    Event evt(WRITE, lpn, 1, 0);
                    evt.set_address(addr);
                    block->write(evt);
                    map[lpn] = addr;
                    if(hcID->is_hot(lpn))///@TODO Fix
                    {
                        hotValidPages[addr.package][addr.die][addr.plane][addr.block]++;
                    }
                    success = true;
                    #if defined DEBUG || defined CHECK_VALID_PAGES
                    check_valid_pages(map.size());
                    check_ftl_integrity();
                    #endif
                }
            }
        }
    }
    #if defined DEBUG || defined CHECK_VALID_PAGES
    check_valid_pages(map.size());
    check_ftl_integrity();
    #endif
}
*/

FtlImpl_DWF::FtlImpl_DWF(Controller &controller, HotColdID *hcID):
    FtlParent(controller), map(),
    hcID(hcID), hotValidPages(SSD_SIZE,  std::vector<std::vector<std::vector<uint> > >(PACKAGE_SIZE, std::vector<std::vector<uint> >(DIE_SIZE, std::vector<uint>(PLANE_SIZE,0))))
{
    return;
}



FtlImpl_DWF::~FtlImpl_DWF(void)
{
    return;
}

enum status FtlImpl_DWF::read(Event &event)
{
    controller.stats.numFTLRead++;
    const uint lpn = event.get_logical_address();
    event.set_address(map[lpn]);

    return SUCCESS;
}


void FtlImpl_DWF::modify_ftl(const ulong lpn, const Address &address)
{
    map[lpn] = address;
}

void FtlImpl_DWF::modify_ftl_page(const ulong lpn, const uint  newPage)
{
    map[lpn].page = newPage;
}

enum status FtlImpl_DWF::write(Event &event)
{
    #if defined DEBUG || defined CHECK_VALID_PAGES
    check_valid_pages(map.size());
    check_ftl_integrity();
    #endif
    controller.stats.numFTLWrite++;
    const ulong lpn = event.get_logical_address();

    ///Invalidate previous page
    get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);
    if(hcID->is_hot(lpn))
    {
        hotValidPages[map[lpn].package][map[lpn].die][map[lpn].plane][map[lpn].block]--;
    }

    while(WFEPtr->get_next_page(WFE) != SUCCESS)//Still space in WFE
    {
        /// Need to select a victim block through GC
        // Temporary address until we find a suitable real victim
        Address  victim =map[lpn];
        Block *victimPtr;
        do {
            garbage->collect(victim);
            victimPtr = controller.get_block_pointer(victim);
        } while (victimPtr == WFEPtr or victimPtr == WFIPtr);

        const uint j = victimPtr->get_pages_valid();
        assert(j <= BLOCK_SIZE);
        const uint k = WFIPtr->get_pages_empty();
        assert(k <= BLOCK_SIZE);

        controller.stats.numGCRead += j;
        controller.stats.numGCErase += 1;
        controller.stats.victimValidDist[j] = controller.stats.victimValidDist[j] + 1;

        if(j <= k) // Sufficient space to copy everything to WFI
        {
            victimPtr->_erase_and_copy(event, WFI, WFIPtr,
                                       [this,&victim](const ulong lpn, const Address &addr) {
                                            modify_ftl(lpn,addr);
                                            if(hcID->is_hot(lpn))
                                            {
                                                hotValidPages[victim.package][victim.die][victim.plane][victim.block]--;
                                                hotValidPages[addr.package][addr.die][addr.plane][addr.block]++;
                                            }
                                        },
                                       [this](const ulong lpn, const uint newPage) { modify_ftl_page(lpn,newPage); }
            );

            const uint hotPages = hotValidPages[WFE.package][WFE.die][WFE.plane][WFE.block];
            #if defined DEBUG || defined CHECK_VALID_PAGES
            check_hot_pages(WFE, WFEPtr, hotPages);
            #endif
            controller.stats.WFEHotPagesDist[hotPages] =  controller.stats.WFEHotPagesDist[hotPages]+1;
            /// Victim replaces the external WF
            WFE = victim;
            WFEPtr =victimPtr;
        }else{
            victimPtr->_erase_and_copy(event, WFI, WFIPtr,
                                       [this,&victim](const ulong lpn, const Address &addr) {
                                            modify_ftl(lpn,addr);
                                            if(hcID->is_hot(lpn))
                                            {
                                                hotValidPages[victim.package][victim.die][victim.plane][victim.block]--;
                                                hotValidPages[addr.package][addr.die][addr.plane][addr.block]++;
                                            }
                                        },
                                       [this](const ulong lpn, const uint newPage) { modify_ftl_page(lpn,newPage); }
            );

            const uint hotPages = hotValidPages[WFI.package][WFI.die][WFI.plane][WFI.block];
            #if defined DEBUG || defined CHECK_VALID_PAGES
            check_hot_pages(WFI, WFIPtr, hotPages);
            #endif

            controller.stats.WFIHotPagesDist[hotPages] =  controller.stats.WFIHotPagesDist[hotPages]+1;
            /// Victim replaces the internal WF
            WFI = victim;
            WFIPtr =victimPtr;
        }

        controller.stats.numGCWrite += j;
        if(controller.stats.get_currentPE() >= victimPtr->get_erases_remaining()){
            controller.stats.next_currentPE();
        }

    }


    event.set_address(WFE);//Tell WF to write to this (next) page
    map[lpn] = WFE;
    if(hcID->is_hot(lpn))
    {
        hotValidPages[WFE.package][WFE.die][WFE.plane][WFE.block]++;
    }
    assert(controller.get_page_pointer(WFE)->get_state() == EMPTY);
    #if defined DEBUG  || defined CHECK_VALID_PAGES
        check_valid_pages(map.size()-1); //1 LPN "missing": the one we are writing next
        check_ftl_integrity(lpn);
    #endif

    return SUCCESS;
}


enum status FtlImpl_DWF::trim(Event &event)
{
    controller.stats.numFTLTrim++;
    ///@TODO Implement
    const uint lpn = event.get_logical_address();
    event.set_address(map[lpn]);
    //get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);//Should we do this here?
    return SUCCESS;
}

void FtlImpl_DWF::check_ftl_integrity(const ulong lpn)
{
    for(ulong l = 0; l < map.size(); l++)
    {
        if(l != lpn)
        {
            Address addr = map[l];
            assert(controller.get_page_pointer(addr)->get_state() == VALID);
            const ulong la = controller.get_page_pointer(addr)->get_logical_address();
            assert(la == l);
        }
    }
    assert(controller.get_page_pointer(WFE)->get_state() == EMPTY);
    for(uint package = 0; package < SSD_SIZE; package++){
        for(uint die = 0; die < PACKAGE_SIZE; die++){
            for(uint plane = 0; plane < DIE_SIZE; plane++){
                for(uint b = 0; b < PLANE_SIZE; b++){
                    for(uint p = 0; p < BLOCK_SIZE; p++){
                        Address addr(package,die,plane,b,p,PAGE);
                        if(controller.get_page_pointer(addr)->get_state() == VALID){
                            const ulong la = controller.get_page_pointer(addr)->get_logical_address();
                            assert(la != lpn);// WAS INVALIDATED!
                            assert(map[la].block == b);
                            assert(map[la].page == p);
                        }
                    }
                }
            }
        }
    }
}

void FtlImpl_DWF::check_ftl_integrity()
{
    for(ulong l = 0; l < map.size(); l++)
    {
        Address addr = map[l];
        assert(controller.get_page_pointer(addr)->get_state() == VALID);
        const ulong la = controller.get_page_pointer(addr)->get_logical_address();
        assert(la == l);
    }
    for(uint package = 0; package < SSD_SIZE; package++){
        for(uint die = 0; die < PACKAGE_SIZE; die++){
            for(uint plane = 0; plane < DIE_SIZE; plane++){
                for(uint b = 0; b < PLANE_SIZE; b++){
                    for(uint p = 0; p < BLOCK_SIZE; p++){
                        Address addr(package,die,plane,b,p,PAGE);
                        if(controller.get_page_pointer(addr)->get_state() == VALID){
                            const ulong la = controller.get_page_pointer(addr)->get_logical_address();
                            assert(map[la].block == b);
                            assert(map[la].page == p);
                        }
                    }
                }
            }
        }
    }
}

void FtlImpl_DWF::check_valid_pages(const ulong numLPN)
{
    uint numPages = 0;
    //std::cout << "NUMPAGES " << event.get_logical_address() << std::endl;
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

void FtlImpl_DWF::check_hot_pages(Address block, Block *blockPtr, const uint hotPages)
{
    uint hotVictimPages = 0;
    for(uint p = 0; p < BLOCK_SIZE; p++){
        block.page = p;
        const Page *page = blockPtr->get_page_pointer(block);
        if(page->get_state() == VALID and hcID->is_hot(page->get_logical_address()))
        {
            hotVictimPages++;
        }
    }
    assert(hotVictimPages == hotPages);
}
