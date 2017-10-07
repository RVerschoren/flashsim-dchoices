/* Copyright 2009, 2010 Brendan Tauras */

/* ssd_event.cpp is part of FlashSim. */

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

/* Event class
 * Brendan Tauras 2010-07-16
 *
 * Class to manage I/O requests as events for the SSD.  It was designed to keep
 * track of an I/O request by storing its type, addressing, and timing.  The
 * SSD class creates an instance for each I/O request it receives.
 */

#include <assert.h>
#include <stdio.h>
#include "ssd.h"
#include <string>
#include <fstream>
#include <sstream>
#include<set>

using namespace ssd;

/* see "enum event_type" in ssd.h for details on event types */
Event::Event(enum event_type type, ulong logical_address, uint size, double start_time):
    start_time(start_time),
    time_taken(0.0),
    bus_wait_time(0.0),
    type(type),
    logical_address(logical_address),
    size(size),
    payload(NULL),
    next(NULL),
    noop(false)
{
    assert(start_time >= 0.0);
    return;
}

Event::~Event(void)
{
    return;
}

/* find the last event in the list to finish and use that event's finish time
 * 	to calculate time_taken
 * add bus_wait_time for all events in the list to bus_wait_time
 * all events in the list do not need to start at the same time
 * bus_wait_time can potentially exceed time_taken with long event lists
 * 	because bus_wait_time is a sum while time_taken is a max
 * be careful to only call this method once when the metaevent is finished */
void Event::consolidate_metaevent(Event &list)
{
    Event *cur;
    double max;
    double tmp;

    assert(start_time >= 0);

    /* find max time taken with respect to this event's start_time */
    max = start_time - list.start_time + list.time_taken;
    for(cur = list.next; cur != NULL; cur = cur -> next)
    {
        tmp = start_time - cur -> start_time + cur -> time_taken;
        if(tmp > max)
            max = tmp;
        bus_wait_time += cur -> get_bus_wait_time();
    }
    time_taken = max;

    assert(time_taken >= 0);
    assert(bus_wait_time >= 0);
    return;
}

ssd::ulong Event::get_logical_address(void) const
{
    return logical_address;
}

void Event::set_logical_address(const ulong newLPN)
{
    this->logical_address = newLPN;
}

const Address &Event::get_address(void) const
{
    return address;
}

const Address &Event::get_merge_address(void) const
{
    return merge_address;
}

const Address &Event::get_log_address(void) const
{
    return log_address;
}

const Address &Event::get_replace_address(void) const
{
    return replace_address;
}

void Event::set_log_address(const Address &address)
{
    log_address = address;
}

ssd::uint Event::get_size(void) const
{
    return size;
}

enum event_type Event::get_event_type(void) const
{
    return type;
}

void Event::set_event_type(const enum event_type &type)
{
    this->type = type;
}

double Event::get_start_time(void) const
{
    assert(start_time >= 0.0);
    return start_time;
}

double Event::get_time_taken(void) const
{

    assert(time_taken >= 0.0);
    return time_taken;
}

double Event::get_bus_wait_time(void) const
{
    assert(bus_wait_time >= 0.0);
    return bus_wait_time;
}

bool Event::get_noop(void) const
{
    return noop;
}

Event *Event::get_next(void) const
{
    return next;
}

void Event::set_payload(void *payload)
{
    this->payload = payload;
}

void *Event::get_payload(void) const
{
    return payload;
}

void Event::set_address(const Address &address)
{
    this -> address = address;
    return;
}

void Event::set_merge_address(const Address &address)
{
    merge_address = address;
    return;
}

void Event::set_replace_address(const Address &address)
{
    replace_address = address;
}

void Event::set_noop(bool value)
{
    noop = value;
}

void Event::set_next(Event &next)
{
    this -> next = &next;
    return;
}

double Event::incr_bus_wait_time(double time_incr)
{
    if(time_incr > 0.0)
        bus_wait_time += time_incr;
    return bus_wait_time;
}

double Event::incr_time_taken(double time_incr)
{
    if(time_incr > 0.0)
        time_taken += time_incr;
    return time_taken;
}

void Event::print(FILE *stream)
{
    if(type == READ)
        fprintf(stream, "Read ");
    else if(type == WRITE)
        fprintf(stream, "Write");
    else if(type == ERASE)
        fprintf(stream, "Erase");
    else if(type == MERGE)
        fprintf(stream, "Merge");
    else
        fprintf(stream, "Unknown event type: ");
    address.print(stream);
    if(type == MERGE)
        merge_address.print(stream);
    fprintf(stream, " Time[%f, %f) Bus_wait: %f\n", start_time, start_time + time_taken, bus_wait_time);
    return;
}

#if 0
/* may be useful for further integration with DiskSim */

/* caution: copies pointers from rhs */
ioreq_event &Event::operator= (const ioreq_event &rhs)
{
    assert(&rhs != NULL);
    if((const ioreq_event *) &rhs == (const ioreq_event *) &(this -> ioreq))
        return *(this -> ioreq);
    ioreq -> time = rhs.time;
    ioreq -> type = rhs.type;
    ioreq -> next = rhs.next;
    ioreq -> prev = rhs.prev;
    ioreq -> bcount = rhs.bcount;
    ioreq -> blkno = rhs.blkno;
    ioreq -> flags = rhs.flags;
    ioreq -> busno = rhs.busno;
    ioreq -> slotno = rhs.slotno;
    ioreq -> devno = rhs.devno;
    ioreq -> opid = rhs.opid;
    ioreq -> buf = rhs.buf;
    ioreq -> cause = rhs.cause;
    ioreq -> tempint1 = rhs.tempint1;
    ioreq -> tempint2 = rhs.tempint2;
    ioreq -> tempptr1 = rhs.tempptr1;
    ioreq -> tempptr2 = rhs.tempptr2;
    ioreq -> mems_sled = rhs.mems_sled;
    ioreq -> mems_reqinfo = rhs.mems_reqinfo;
    ioreq -> start_time = rhs.start_time;
    ioreq -> batchno = rhs.batchno;
    ioreq -> batch_complete = rhs.batch_complete;
    ioreq -> batch_size = rhs.batch_size;
    ioreq -> batch_next = rhs.batch_next;
    ioreq -> batch_prev = rhs.batch_prev;
    return *ioreq;
}
#endif

void Event::set_hot(const bool isHot)
{
    hot = isHot;
}

bool Event::is_hot() const
{
    return hot;
}
/*
void ssd::read_oracle(const std::string &filename, std::vector<Event> &events)
{
    std::ifstream data(filename);
    std::vector<bool> oracle;
    std::string line;
    while(std::getline(data,line))
    {
        std::stringstream  lineStream(line);
        std::string        cell;
        std::getline(lineStream,cell,',');
        const bool value = std::stoi(cell) != 0;
        oracle.push_back(value);
    }
    //return oracle;
    std::cout << filename << "    "  << oracle.size() << "    " << events.size() << std::endl;
    assert(oracle.size() == events.size());
    for(ulong it = 0; it < events.size(); it++)
    {
        events[it].set_hot(oracle[it]);
    }
}
*/
/**
 * @brief Reads events from a trace file
 * @param filename Filename
 * @param readLine Function to convert a line of the tracefile to an ssd::Event
 * @param events Vector of ssd::Events, memory needs to be preallocated!
 *//*
void ssd::read_event_from_trace(std::string filename, std::function<Event (std::string)> readLine, std::vector<Event> &events)
{
    std::ifstream data(filename);
    std::string line;
    unsigned long it = 0;
    while(std::getline(data,line))
    {
        events.at(it) = readLine(line);
    }
    return events;
}

Event ssd::read_event_simple(std::string line)
{
    const char delim = ',';
    std::stringstream  lineStream(line);
    std::string        cell;
    std::getline(lineStream,cell,delim);
    const unsigned long startAddress = cell.empty()? 0UL : std::stoul(cell);
    std::getline(lineStream,cell,delim);
    const  event_type  type = (cell.empty() or std::stoi(cell) != 0)? WRITE : TRIM;
    ///@TODO Find a better solution than zero start times to determine the right start time
    return Event(type, startAddress, 1, 0.0);
}

Event ssd::read_event_BIOtracer(std::string line)
{
    const char delim = '\t';
    std::stringstream  lineStream(line);
    std::string        cell;

    std::getline(lineStream,cell,delim);
    const unsigned long startAddress = cell.empty()? 0UL : std::stoul(cell);
    //Need to getline twice because fields are delimited by 2 tabs instead of only 1...
    std::getline(lineStream,cell,delim);
    std::getline(lineStream,cell,delim);
    //const unsigned long numSectors = cell.empty()? 1UL : (std::stoul(cell)/8+1);
    std::getline(lineStream,cell,delim);
    std::getline(lineStream,cell,delim);
    const unsigned long numSectors = cell.empty()? 1UL : (std::stoul(cell)/4096+1);//in bytes
    std::getline(lineStream,cell,delim);
    std::getline(lineStream,cell,delim);
    const unsigned int value = cell.empty()? 0UL : std::stoul(cell);
    const event_type type = value % 2 == 0? READ : WRITE;
    std::getline(lineStream,cell,delim);
    std::getline(lineStream,cell,delim);
    //const double requestGenTime= cell.empty()? 0.0 : (std::stod(cell));
    std::getline(lineStream,cell,delim);
    std::getline(lineStream,cell,delim);
    //const double requestProcessTime= cell.empty()? 0.0 : (std::stod(cell));
    std::getline(lineStream,cell,delim);
    std::getline(lineStream,cell,delim);
    const double startTime = cell.empty()? 0.0 : std::stod(cell);

    return Event(type, startAddress, numSectors, startTime);
}

ulong ssd::count_unique_lpns(const std::vector<Event> &events)
{
    std::set<ulong> uniqueLPNs;
    for(const Event &evt : events)
    {
        const ulong &lpn = evt.get_logical_address();
        if(uniqueLPNs.find(lpn) == uniqueLPNs.end()) // Not found in set
        {
            uniqueLPNs.insert(lpn);
        }
    }
    return uniqueLPNs.size();
}*/
