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

#include <iostream>

using namespace ssd;

void FtlImpl_SWF::initialize(const ulong numLPN)
{
    // Just assume for now that we have PAGE validity, we'll check it later anyway
    Address addr(0,0,0,0,0,PAGE);
    WF = addr;
    WFPtr = controller.get_block_pointer(WF);
    for(unsigned int lpn = 0; lpn < numLPN; lpn++){

        bool success = false;
        while(not success){

            addr.package = RandNrGen::getInstance().get(SSD_SIZE);
            addr.die = RandNrGen::getInstance().get(PACKAGE_SIZE);
            addr.plane = RandNrGen::getInstance().get(DIE_SIZE);
            addr.block = RandNrGen::getInstance().get(PLANE_SIZE);
            assert(addr.check_valid() >= BLOCK);

            Block * block = controller.get_block_pointer(addr);

            if(block != WFPtr and block->get_next_page(addr) == SUCCESS){
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


FtlImpl_SWF::FtlImpl_SWF(Controller &controller):
    FtlParent(controller), maxLBA((1.0-SPARE_FACTOR)*BLOCK_SIZE*PLANE_SIZE*DIE_SIZE*PACKAGE_SIZE), map(maxLBA)
{
    return;
}

FtlImpl_SWF::~FtlImpl_SWF(void)
{
    return;
}

enum status FtlImpl_SWF::read(Event &event)
{
    controller.stats.numFTLRead++;
    const uint lpn = event.get_logical_address();
    event.set_address(map[lpn]);

    return SUCCESS;
    //return controller.issue(event);
}

enum status FtlImpl_SWF::write(Event &event)
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
        assert(numPages == maxLBA);
    #endif
    controller.stats.numFTLWrite++;
    const ulong lpn = event.get_logical_address();

    ///Invalidate previous page
    Address phaddr = map[lpn];
    ///@TODO This shouldn't be necessary FIX THIS
    ///phaddr.valid = PAGE;//This shouldn't be necessary FIX THIS
    assert(phaddr.valid >= PAGE);
    controller.get_block_pointer(phaddr)->invalidate_page(phaddr.page);
    assert(controller.get_page_pointer(phaddr)->get_state() == INVALID);

    while(WFPtr->get_next_page(WF) != SUCCESS)//Still space in WF
    {
        //std::cout << "WRITE FRONTIER " << controller.get_num_valid(WF) + controller.get_num_invalid(WF) << "/" << BLOCK_SIZE << std::endl;
        //Need to select a victim block through GC
        Address victim = map[lpn];
        garbage->collect(victim);


        double startTime = event.get_start_time();
        std::vector<ulong> lpns;
        for(uint p = 0; p < BLOCK_SIZE; p++){
            //Pretend to read the data to copy it
            Event readEvent(READ, event.get_logical_address(), 1, startTime);
            victim.page = p;
            victim.valid = PAGE;
            readEvent.set_address(victim);

            if(controller.issue(readEvent) == SUCCESS){//Page is valid?
                event.incr_time_taken(readEvent.get_time_taken());
                startTime += readEvent.get_time_taken();

                //std::cout << victim.block << " victim read " << victim.page  << " VALIDITY " << (victim.check_valid() >= BLOCK) << std::endl;
                assert(victim.valid >= PAGE);
                lpns.push_back(controller.get_page_pointer(victim)->get_logical_address());//Sneakily get the LPN behind the controller's back
            }
        }
        controller.stats.numGCRead += lpns.size();

        //std::cout << "GC SELECT " << victim.block << " VALIDITY " << controller.get_num_valid(victim) << "/" << BLOCK_SIZE << "     " << lpns.size() << std::endl;
        /// Erase the victim block
        Event eraseEvent(ERASE, event.get_logical_address(), 1, event.get_start_time());
        eraseEvent.set_address(victim);

        if (controller.issue(eraseEvent) == FAILURE) printf("Erase failed");
        event.incr_time_taken(eraseEvent.get_time_taken());
        if(GC_FULLVICTIM_STATS or lpns.size() < BLOCK_SIZE) controller.stats.numGCErase++;

        /// Set the new WF
        WF = victim;
        WFPtr = controller.get_block_pointer(WF);

        /// Copy everything back!
        for(uint p = 0; p < lpns.size(); p++){
            //Pretend to read the data to copy it
            Event copyEvent(WRITE, lpns[p], 1, startTime);
            if(WFPtr->get_next_page(WF) == SUCCESS/* and controller.get_page_pointer(WF)->get_state() == EMPTY*/){
                assert(controller.get_page_pointer(WF)->get_state() == EMPTY);
                copyEvent.set_address(WF);

                if(controller.issue(copyEvent) == SUCCESS){
                    event.incr_time_taken(copyEvent.get_time_taken());
                    startTime += copyEvent.get_time_taken();
                    map[lpns[p]] = WF;
                }else{
                    assert(false);
                    return FAILURE;
                }
            }else{
                assert(false);
                return FAILURE;
            }
        }
        controller.stats.numGCWrite += lpns.size();
        const ulong numErasesOfVictim = BLOCK_ERASES - WFPtr->get_erases_remaining();
        if(controller.stats.get_currentPE() < numErasesOfVictim){
            controller.stats.next_currentPE();
        }
    }


    event.set_address(WF);//Tell WF to write to this (next) page
    map[lpn] = WF;

    #ifndef NDEBUG
        numPages = 0;
        std::cout << "NUMPAGES BEFORE WRITE " << event.get_logical_address() << std::endl;
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
        assert(numPages == maxLBA-1);
        assert(controller.get_page_pointer(WF)->get_state() == EMPTY);
        for(uint l = 0; l < maxLBA; l++){
            if(l != lpn)
            {
                Address addr = map[l];
                assert(controller.get_page_pointer(addr)->get_state() == VALID);
                const ulong la = controller.get_page_pointer(addr)->get_logical_address();
                assert(la == l);
            }
        }
        assert(controller.get_page_pointer(WF)->get_state() == EMPTY);
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
    #endif

    return SUCCESS;
}

enum status FtlImpl_SWF::trim(Event &event)
{
    controller.stats.numFTLTrim++;
    ///@TODO Implement
    const uint lpn = event.get_logical_address();
    event.set_address(map[lpn]);
    get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);//Should we do this here?
    return SUCCESS;
}

