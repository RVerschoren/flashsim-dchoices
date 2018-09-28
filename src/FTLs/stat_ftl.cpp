/* Copyright 2018 Robin Verschoren */

/* stat_ftl.cpp  */

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

/* Implements Static Drive Partitioning (STAT) (Van Houdt, 2014) */

#include "ssd.h"
#include "util.h"

using namespace ssd;

void
FtlImpl_STAT::initialize(const ulong numLPN)
{
        // Just assume for now that we have PAGE validity
        Address addr(0, 0, 0, 0, 0, PAGE);
        HWF = addr;

        /// TODO  Review how to determine first cold block when not assuming 1 plane
        const double f = HOT_FRACTION;
        maxHotBlocks = static_cast<uint>(std::ceil(f * PLANE_SIZE)) + 1;
        const uint numColdBlocks = PLANE_SIZE - maxHotBlocks;
        CWF = Address(0, 0, 0, maxHotBlocks, 0, PAGE);

        numHotBlocks = 0;
        /// Set hotness
        for (uint package = 0; package < SSD_SIZE; package++) {
                for (uint die = 0; die < PACKAGE_SIZE; die++) {
                        for (uint plane = 0; plane < DIE_SIZE; plane++) {
                                for (uint block = 0; block < PLANE_SIZE; block++) {
                                        const Address blockAddr(package, die, plane, block, 0,
                                                                BLOCK);
                                        set_block_hotness(blockAddr, block < maxHotBlocks);
                                        if (block < maxHotBlocks)
                                                numHotBlocks++;
                                }
                        }
                }
        }
        set_block_hotness(HWF, true);
        set_block_hotness(CWF, false);
        assert(numHotBlocks <= maxHotBlocks);
        assert(numHotBlocks <= PLANE_SIZE);

        /// Initialize at random
        for (unsigned int lpn = 0; lpn < numLPN; lpn++) {
                bool success = false;
                const bool lpnIsHot = hcID->is_hot(lpn);
                while (not success) {
#ifndef SINGLE_PLANE
                        addr.package = RandNrGen::get(SSD_SIZE);
                        addr.die = RandNrGen::get(PACKAGE_SIZE);
                        addr.plane = RandNrGen::get(DIE_SIZE);
#endif
                        if (lpnIsHot) {
                                addr.block = RandNrGen::get(maxHotBlocks);
                        } else {
                                addr.block = maxHotBlocks + RandNrGen::get(numColdBlocks);
                        }
                        assert(addr.check_valid() >= BLOCK);
                        Block* block = get_block(addr);
                        if (not HWF.same_block(addr) and not CWF.same_block(addr) and
                                get_next_page(addr) == SUCCESS) {
                                Event evt(WRITE, lpn, 1, 0);
                                evt.set_address(addr);
                                block->write(evt);
                                map.push_back(addr);
                                success = true;
                        }
                }
        }
#ifdef DEBUG
        check_valid_pages(numLPN);
        check_block_hotness();
        check_ftl_hotness_integrity();
#endif
}

FtlImpl_STAT::FtlImpl_STAT(Controller& controller, HotColdID* hcID, const double p)
        : FtlParent(controller)
        , p(p) 
        , map()
        , hcID(hcID)
{
        return;
}

FtlImpl_STAT::~FtlImpl_STAT(void)
{
        return;
}

enum status
FtlImpl_STAT::read(Event& event) {
        controller.stats.numFTLRead++;
        const ulong lpn = event.get_logical_address();
        event.set_address(map[lpn]);

        return SUCCESS;
}

enum status
FtlImpl_STAT::write(Event& event) {
#ifdef DEBUG
        check_valid_pages(map.size());
        check_block_hotness();
        check_ftl_hotness_integrity();
#endif
        const enum status prewritewear =
        wlvl->prewrite(event, controller, { HWF, CWF });

        controller.stats.numFTLWrite++;
        const ulong lpn = event.get_logical_address();

        /// Invalidate previous page
        get_block(map[lpn])->invalidate_page(map[lpn].page, event.get_start_time() + event.get_time_taken());

        const bool lpnIsHot = hcID->is_hot(lpn);
        // Locate in same plane
        Address planeAddress = (map[lpn]);
        Address beginBlock(planeAddress);
        beginBlock.block = lpnIsHot? 0 : maxHotBlocks;
        Address endBlock(planeAddress);
        endBlock.block = lpnIsHot? (maxHotBlocks-1) : PLANE_SIZE;

        std::pair<Address,Address> range(beginBlock, endBlock);

        while ((lpnIsHot and get_next_page(HWF) != SUCCESS) or
                (not lpnIsHot and get_next_page(CWF) != SUCCESS)) // Still space in WF
        {
            // Need to select a victim block through GC
            Address victim = map[lpn];
            const std::vector<Address> currentWF = { HWF, CWF };

            if (wlvl->suggest_WF(victim, currentWF) == FAILURE) {
                    garbage->collect(event, victim, currentWF, range);
            }
            Block* victimPtr = get_block(victim);
            const bool victimIsHot = get_block_hotness(victim);
            assert(lpnIsHot == victimIsHot);

            const uint j = get_pages_valid(victim);
            assert(j <= BLOCK_SIZE);

            if(lpnIsHot){
                victimPtr->_erase_and_copy(
                    event, HWF, get_block(HWF), // Copy to self
                [this, &victim](const ulong lpn, const Address& addr) {
                        map[lpn] = addr;
                },
                [this](const ulong lpn, const uint newPage) {
                        map[lpn].page = newPage;
                });
                HWF = victim;
            }else{
                victimPtr->_erase_and_copy(
                    event, CWF, get_block(CWF), // Copy to self
                [this, &victim](const ulong lpn, const Address& addr) {
                        map[lpn] = addr;
                },
                [this](const ulong lpn, const uint newPage) {
                        map[lpn].page = newPage;
                });
                CWF = victim;
            }

            controller.stats.erase_block(j, victimIsHot);
                if (controller.stats.get_currentPE() >=
                        victimPtr->get_erases_remaining()) {
                        controller.stats.next_currentPE();
                }
        }
        if (lpnIsHot)
        {
                event.set_address(HWF); // Tell WF to write to this (next) page
                map[lpn] = HWF;
        } else
        {
                event.set_address(CWF); // Tell WF to write to this (next) page
                map[lpn] = CWF;
        }

#ifdef DEBUG
        check_valid_pages(map.size() - 1);
#endif

        return SUCCESS;
}

enum status
FtlImpl_STAT::trim(Event& event) {
        /// TODO Implement
        const ulong lpn = event.get_logical_address();
        event.set_address(map[lpn]);
        // get_block_pointer(map[lpn])->invalidate_page(map[lpn].page);//Should we
        // do this here?
        return SUCCESS;
}

void
FtlImpl_STAT::modifyFTL(const ulong lpn, const Address& address)
{
        map[lpn] = address;
}

void
FtlImpl_STAT::check_valid_pages(const ulong numLPN)
{
        uint numPages = 0;
        for (uint package = 0; package < SSD_SIZE; package++) {
                for (uint die = 0; die < PACKAGE_SIZE; die++) {
                        for (uint plane = 0; plane < DIE_SIZE; plane++) {
                                for (uint b = 0; b < PLANE_SIZE; b++) {
                                        // std::cout << "    BLOCK" << b << std::endl;
                                        for (uint p = 0; p < BLOCK_SIZE; p++) {
                                                Address addr(package, die, plane, b, p, PAGE);
                                                if (controller.get_page_pointer(addr)->get_state() ==
                                                        VALID) {
                                                        numPages++;
                                                }
                                        }
                                }
                        }
                }
        }
        assert(numPages == numLPN);
}

void
FtlImpl_STAT::check_block_hotness()
{
        uint numBlocks = 0;
        for (uint package = 0; package < SSD_SIZE; package++) {
                for (uint die = 0; die < PACKAGE_SIZE; die++) {
                        for (uint plane = 0; plane < DIE_SIZE; plane++) {
                                for (uint b = 0; b < PLANE_SIZE; b++) {
                                        const Address blockAddr(package, die, plane, b, 0, BLOCK);
                                        if (get_block_hotness(blockAddr))
                                                numBlocks++;
                                }
                        }
                }
        }
        assert(numBlocks == numHotBlocks);
}

void
FtlImpl_STAT::check_ftl_hotness_integrity()
{
        for (uint it = 0; it < map.size(); it++) {
                const ulong lpn = it;
                const Address& addr = map[it];
                assert(get_block_hotness(addr) == hcID->is_hot(lpn));
        }
}
