/* Copyright 2017 Robin Verschoren */

/* warm_hcid.cpp  */

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

/* Implements the Write-Awareness Retention Management (WARM) policy from Y. Luo et al. */

#include <queue>
#include "../ssd.h"

using namespace ssd;

WARM_HCID::WARM_HCID(const uint maxHotQueueLength, const uint maxCooldownLength)
    : hotQueue(maxHotQueueLength), cooldownWindow(maxCooldownLength)
{
}


WARM_HCID::~WARM_HCID()
{
}

bool WARM_HCID::is_hot(const ulong lpn) const
{
    return hotSet.find(lpn) != hotSet.end();
}
/*
bool WARM_HCID::request_lpn(const ulong lpn)
{
    assert(hotSet.size() == hotQueue.size());
    assert(cooldownSet.size() == cooldownWindow.size());
    assert(hotQueue.size() <= maxNumHot);
    assert(cooldownWindow.size() <= maxNumCooldown);
    assert(hotQueue.size() >= 0);
    assert(cooldownWindow.size() >= 0);

    bool lpnIsHot = false;

    std::set<ulong>::iterator hotsElem = hotSet.find(lpn);
    if(hotsElem != hotSet.end())
    {
        for(std::deque<ulong>::iterator it = hotQueue.begin(); it != hotQueue.end(); it++)
        {
            if(*it == lpn)
            {
                hotQueue.erase(it);
                break;
            }
        }
        hotQueue.push_front(lpn);
        lpnIsHot = true;

    }else{

       std::set<ulong>::iterator cdsElem = cooldownSet.find(lpn);
        if(cdsElem != cooldownSet.end()) // LPN in cooldown
        {
            for(std::deque<ulong>::iterator it = cooldownWindow.begin(); it != cooldownWindow.end(); it++)
            {
                if(*it == lpn)
                {
                    cooldownWindow.erase(it);
                    cooldownSet.erase(cdsElem);
                    break;
                }
            }
            if(hotQueue.size() == maxNumHot){
                const ulong &endElem =  hotQueue.back(); // Remove first, so we stay within capacity
                hotQueue.pop_back();
                hotSet.erase(endElem);
            }
            hotSet.insert(lpn);
            hotQueue.push_front(lpn);
            lpnIsHot = true;

        }else{
            if(cooldownWindow.size() == maxNumCooldown)
            {
                const ulong &endElem =  cooldownWindow.back(); // Remove first, so we stay within capacity
                cooldownWindow.pop_back();
                cooldownSet.erase(endElem);
            }
            cooldownWindow.push_front(lpn);
            cooldownSet.insert(lpn);
            lpnIsHot = false;
        }
    }
    assert(hotSet.size() == hotQueue.size());
    assert(cooldownSet.size() == cooldownWindow.size());
    assert(hotQueue.size() <= maxNumHot);
    assert(cooldownWindow.size() <= maxNumCooldown);
    assert(hotQueue.size() >= 0);
    assert(cooldownWindow.size() >= 0);
    return lpnIsHot;
}*/

