/* Copyright 2017 Robin Verschoren*/

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

/* EventReader class
 * Robin Verschoren
 *
 * Class to manage read requests as events for the SSD.
 */

#include <assert.h>
#include <stdio.h>
#include "ssd.h"
#include <string>
#include <fstream>
#include <sstream>

using namespace ssd;

EventReader::EventReader(const std::string traceFileName, const ulong numEvents, const EVENT_READER_MODE mode)
    : traceFileName(traceFileName), currentEvent(0), numEvents(numEvents), readMode(mode), oracleFileName(oracleFileName), traceStream(traceFileName),  usingOracle(false)
{
}

EventReader::EventReader(const std::string traceFileName, const ulong numEvents, const EVENT_READER_MODE mode, const std::string oracleFileName)
    : traceFileName(traceFileName), currentEvent(0), numEvents(numEvents), readMode(mode), oracleFileName(oracleFileName), traceStream(traceFileName), oracleStream(oracleFileName), usingOracle(true)
{
}

/*
void EventReader::read_oracle(const std::string &filename, std::vector<Event> &events)
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
    assert(oracle.size() == events.size());
    for(ulong it = 0; it < events.size(); it++)
    {
        events[it].set_hot(oracle[it]);
    }
}*/
/**
 * @brief Reads events from a trace file
 * @param filename Filename
 * @param readLine Function to convert a line of the tracefile to an ssd::Event
 * @return Vector of ssd::Events
 */
std::vector<Event> EventReader::read_events_from_trace(const std::string &filename) const
{
    std::ifstream data(filename);
    std::string line;
    std::vector<Event> events;
    while(std::getline(data,line))
    {
        events.push_back(read_event(line));
    }
    return events;
}

bool EventReader::read_next_oracle()
{
    std::string line;
    while(std::getline(oracleStream, line))
    {
        std::stringstream  lineStream(line);
        std::string cell;
        std::getline(lineStream, cell, ',');
        return std::stoi(cell) != 0;
    }
}

Event EventReader::read_next_event()
{
    if(currentEvent == numEvents) traceStream.seekg(0);
    std::string line;
    std::getline(traceStream, line);
    Event evt = read_event(line);
    if(usingOracle) evt.set_hot(read_next_oracle());
    return evt;
}

Event EventReader::read_event(const std::string &line) const
{
    switch (readMode)
    {
        case EVTRDR_SIMPLE:
            return read_event_simple(line);
            break;
        case EVTRDR_BIOTRACER:
            return read_event_BIOtracer(line);
            break;
        default:
            return read_event_simple(line);
    }
}

Event EventReader::read_event_simple(const std::string &line) const
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

Event EventReader::read_event_BIOtracer(const std::string &line) const
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

std::set<ulong> EventReader::read_accessed_lpns() const
{
    std::set<ulong> uniqueLPNs;
    /// Keep this separate from currentEvent/traceStream
    std::ifstream data(traceFileName);
    std::string line;
    while(std::getline(data, line))
    {
        uniqueLPNs.insert(read_event(line).get_logical_address());
    }
    return uniqueLPNs;
}

std::set<ulong> EventReader::read_hot_lpns() const
{
    std::set<ulong> hotLPNs;
    if(not usingOracle) return hotLPNs;

    /// Keep this separate from currentEvent/traceStream
    std::ifstream data(traceFileName);
    std::ifstream oracleData(oracleFileName);
    std::string line;
    std::string oracleLine;
    std::string cell;
    while(std::getline(data, line))
    {
        std::getline(oracleData, oracleLine);
        std::stringstream  lineStream(oracleLine);
        std::getline(lineStream, cell, ',');
        if(std::stoi(cell) != 0) hotLPNs.insert(read_event(line).get_logical_address());
    }
    return hotLPNs;
}
