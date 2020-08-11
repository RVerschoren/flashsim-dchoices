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

#include "ssd.h"
#include <assert.h>
#include <fstream>
#include <limits>
#include <sstream>

using namespace ssd;

EventReader::EventReader(const std::string traceFileName, const ulong numEvents, const EVENT_READER_MODE mode)
	: readMode(mode), traceFileName(traceFileName), oracleFileName(""), currentEvent(0), numEvents(numEvents),
	  usingOracle(false), traceStream(traceFileName)
{
}

EventReader::EventReader(const std::string traceFileName, const ulong numEvents, const EVENT_READER_MODE mode,
						 const std::string oracleFileName)
	: readMode(mode), traceFileName(traceFileName), oracleFileName(oracleFileName), currentEvent(0),
	  numEvents(numEvents), usingOracle(true), traceStream(traceFileName), oracleStream(oracleFileName)
{
}

/*
void EventReader::read_oracle(const std::string &filename, std::vector<Event>
&events)
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
std::vector<Event> EventReader::read_events_from_trace(const std::string& filename) const
{
	std::ifstream data(filename);
	std::string line;
	std::vector<Event> events;
	while (std::getline(data, line)) {
		read_event(line, events);
	}
	return events;
}

std::vector<IOEvent> EventReader::read_IO_events_from_trace(const std::string& filename) const
{
	std::ifstream data(filename);
	std::string line;
	std::vector<IOEvent> events;
	while (std::getline(data, line)) {
		read_IO_event(line, events);
	}
	return events;
}

bool EventReader::read_next_oracle()
{
	std::string line;
	while (std::getline(oracleStream, line)) {
		std::stringstream lineStream(line);
		std::string cell;
		std::getline(lineStream, cell, ',');
		return std::stoi(cell) != 0;
	}
	return false;
}

Event EventReader::read_next_event()
{
	if (currentEvent == numEvents) {
		traceStream.seekg(0);
		currentEvent = 0;
	}
	std::string line;
	std::getline(traceStream, line);
	std::vector<Event> events;
	read_event(line, events);
	if (usingOracle)
		events[0].set_hot(read_next_oracle());
	return events[0];
}

void EventReader::read_IO_event(const std::string& line, std::vector<IOEvent>& events) const
{
	switch (readMode) {
	case EVTRDR_SIMPLE:
		return read_IO_event_simple(line, events);
		break;
	case EVTRDR_BIOTRACER:
		return read_IO_event_BIOtracer(line, events);
		break;
	default:
		return read_IO_event_simple(line, events);
	}
}

void EventReader::read_IO_event_simple(const std::string& line, std::vector<IOEvent>& events) const
{
	const char delim = ',';
	std::stringstream lineStream(line);
	std::string cell;
	std::getline(lineStream, cell, delim);
	const unsigned long startAddress = cell.empty() ? 0UL : std::stoul(cell);
	std::getline(lineStream, cell, delim);
	const event_type type = (cell.empty() or std::stoi(cell) == 0) ? READ : WRITE;
	/// FIXME ORIGINAL const event_type type = (cell.empty() or std::stoi(cell) != 0) ? WRITE : TRIM;
	events.push_back(IOEvent(type, startAddress, 1));
}

void EventReader::read_IO_event_BIOtracer(const std::string& line, std::vector<IOEvent>& events) const
{
	const char delim = '\t';
	std::stringstream lineStream(line);
	std::string cell;

	std::getline(lineStream, cell, delim);
	unsigned long startAddress = cell.empty() ? 0UL : std::stoul(cell);
	// Need to getline twice because fields are delimited by 2 tabs instead of
	// only 1...
	std::getline(lineStream, cell, delim);
	std::getline(lineStream, cell, delim);
	// const unsigned long numSectors = cell.empty()? 1UL :
	// (std::stoul(cell)/8+1);
	std::getline(lineStream, cell, delim);
	std::getline(lineStream, cell, delim);
	// const unsigned long numSectors = cell.empty()? 1UL :
	// (std::stoul(cell)/4096+1);//in bytes
	long numBytes = cell.empty() ? 4095L : std::stol(cell); // in bytes
	std::getline(lineStream, cell, delim);
	std::getline(lineStream, cell, delim);
	const unsigned long value = cell.empty() ? 0UL : std::stoul(cell);
	const event_type type = value % 2 == 0 ? READ : WRITE;
	do {
		events.push_back(IOEvent(type, startAddress, 1));
		startAddress++;
		numBytes = numBytes - 4096;
	} while (numBytes > 0);
	// return IOEvent(type, startAddress, numSectors);
}

std::string EventReader::write_event(const Event& e) const
{
	switch (readMode) {
	case EVTRDR_SIMPLE:
		return write_event_simple(e);
		break;
	case EVTRDR_BIOTRACER:
		return write_event_BIOtracer(e);
		break;
	default:
		return write_event_simple(e);
	}
}

void EventReader::read_event(const std::string& line, std::vector<Event>& events) const
{
	switch (readMode) {
	case EVTRDR_SIMPLE:
		return read_event_simple(line, events);
		break;
	case EVTRDR_BIOTRACER:
		return read_event_BIOtracer(line, events);
		break;
	default:
		return read_event_simple(line, events);
	}
}

void EventReader::read_event_simple(const std::string& line, std::vector<Event>& events) const
{
	const char delim = ',';
	std::stringstream lineStream(line);
	std::string cell;
	std::getline(lineStream, cell, delim);
	const unsigned long startAddress = cell.empty() ? 0UL : std::stoul(cell);
	std::getline(lineStream, cell, delim);
	// const event_type type = (cell.empty() or std::stoi(cell) != 0) ? WRITE : TRIM;
	const event_type type = (cell.empty() or std::stoi(cell) == 0) ? READ : WRITE;
	/// FIXME Find a better solution than zero start times to determine the right start time
	// return Event(type, startAddress, 1, 0.0);
	events.push_back(Event(type, startAddress, 1, 0.0));
}

std::string EventReader::write_event_simple(const Event& evt) const
{
	const std::string typeStr = (evt.get_event_type() == TRIM) ? "0" : "1";
	return (std::to_string(evt.get_logical_address()) + "," + typeStr);
}

void EventReader::read_event_BIOtracer(const std::string& line, std::vector<Event>& events) const
{
	const char delim = '\t';
	std::stringstream lineStream(line);
	std::string cell;

	std::getline(lineStream, cell, delim);
	unsigned long startAddress = cell.empty() ? 0UL : std::stoul(cell);
	// Need to getline twice because fields are delimited by 2 tabs instead of
	// only 1...
	std::getline(lineStream, cell, delim);
	std::getline(lineStream, cell, delim);
	// const unsigned long numSectors = cell.empty()? 1UL :
	// (std::stoul(cell)/8+1);
	std::getline(lineStream, cell, delim);
	std::getline(lineStream, cell, delim);
	// const unsigned long numSectors = cell.empty()? 1UL :
	// (std::stoul(cell)/4096+1);//in bytes
	long numBytes = cell.empty() ? 4096L : std::stol(cell); // in bytes
	std::getline(lineStream, cell, delim);
	std::getline(lineStream, cell, delim);
	const unsigned long value = cell.empty() ? 0UL : std::stoul(cell);
	const event_type type = value % 2 == 0 ? READ : WRITE;
	std::getline(lineStream, cell, delim);
	std::getline(lineStream, cell, delim);
	// const double requestGenTime= cell.empty()? 0.0 : (std::stod(cell));
	std::getline(lineStream, cell, delim);
	std::getline(lineStream, cell, delim);
	// const double requestProcessTime= cell.empty()? 0.0 : (std::stod(cell));
	std::getline(lineStream, cell, delim);
	std::getline(lineStream, cell, delim);
	const double startTime = cell.empty() ? 0.0 : std::stod(cell);
	do {
		events.push_back(Event(type, startAddress, 1, startTime));
		startAddress++;
		numBytes = numBytes - 4096;
	} while (numBytes > 0);
	// return Event(type, startAddress, numSectors, startTime);
}

std::string EventReader::write_event_BIOtracer(const Event& evt) const
{
	const std::string delim = "\t\t";
	const std::string emptyCell = "0";
	const std::string typeStr = (evt.get_event_type() == READ) ? "0" : "1";
	return (std::to_string(evt.get_logical_address()) + delim + std::to_string(evt.get_size() * 8) + delim +
			std::to_string(evt.get_size() * 4096) + delim + typeStr + delim + emptyCell + delim + emptyCell + delim +
			std::to_string(evt.get_start_time()));
}

ulong EventReader::find_max_lpn(const std::vector<IOEvent>& events) const
{
	ulong maxLPN = -std::numeric_limits<ulong>::infinity();
	for (const IOEvent& evt : events) {
		maxLPN = std::max(maxLPN, evt.lpn);
	}
	return maxLPN;
}
