/* Copyright 2009, 2010 Brendan Tauras */

/* ssd.h is part of FlashSim. */

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

/* ssd.h
 * Brendan Tauras 2010-07-16
 * Main SSD header file
 * 	Lists definitions of all classes, structures,
 * 		typedefs, and constants used in ssd namespace
 *		Controls options, such as debug asserts and test code insertions
 */
#include "util.h"
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#ifndef _SSD_H
#define _SSD_H
namespace ssd
{

/* define exit codes for errors */
#define MEM_ERR -1
#define FILE_ERR -2

/* Uncomment to disable asserts for production */
//#define NDEBUG

/* some obvious typedefs for laziness */
typedef unsigned int uint;
typedef unsigned long ulong;

/* Simulator configuration from ssd_config.cpp */

/* Configuration file parsing for extern config variables defined below */
void load_entry(char* name, double value, uint line_number);
void load_config(void);
void print_config(FILE* stream);

/* Ram class:
 * 	delay to read from and write to the RAM for 1 page of data */
extern const double RAM_READ_DELAY;
extern const double RAM_WRITE_DELAY;

/* Bus class:
 * 	delay to communicate over bus
 * 	max number of connected devices allowed
 * 	flag value to detect free table entry (keep this negative)
 * 	number of time entries bus has to keep track of future schedule usage
 * 	number of simultaneous communication channels - defined by SSD_SIZE */
extern const double BUS_CTRL_DELAY;
extern const double BUS_DATA_DELAY;
extern const uint BUS_MAX_CONNECT;
extern const double BUS_CHANNEL_FREE_FLAG;
extern const uint BUS_TABLE_SIZE;
/* extern const uint BUS_CHANNELS = 4; same as # of Packages, defined by
 * SSD_SIZE */

/* Ssd class:
 * 	number of Packages per Ssd (size) */
extern const uint SSD_SIZE;

/* Package class:
 * 	number of Dies per Package (size) */
extern const uint PACKAGE_SIZE;

/* Die class:
 * 	number of Planes per Die (size) */
extern const uint DIE_SIZE;

/* Plane class:
 * 	number of Blocks per Plane (size)
 * 	delay for reading from plane register
 * 	delay for writing to plane register
 * 	delay for merging is based on read, write, reg_read, reg_write
 * 		and does not need to be explicitly defined */
/// TODO Reset to const
/// extern const uint PLANE_SIZE;
extern uint PLANE_SIZE;
extern const double PLANE_REG_READ_DELAY;
extern const double PLANE_REG_WRITE_DELAY;

/* Block class:
 * 	number of Pages per Block (size)
 * 	number of erases in lifetime of block
 * 	delay for erasing block */
/// TODO Reset to const
/// extern const uint BLOCK_SIZE;
extern uint BLOCK_SIZE;
// extern  const uint BLOCK_ERASES;
extern uint BLOCK_ERASES;
extern const double BLOCK_ERASE_DELAY;

/* Page class:
 * 	delay for Page reads
 * 	delay for Page writes */
extern const double PAGE_READ_DELAY;
extern const double PAGE_WRITE_DELAY;
extern const uint PAGE_SIZE;
extern const bool PAGE_ENABLE_DATA;

/*
 * Mapping directory
 */
extern const uint MAP_DIRECTORY_SIZE;

/*
 * FTL Implementation
 */
/// TODO Reset to const
/// extern const uint FTL_IMPLEMENTATION;
extern uint FTL_IMPLEMENTATION;

/*
 * LOG page limit for BAST.
 */
extern const uint BAST_LOG_PAGE_LIMIT;

/*
 * LOG page limit for FAST.
 */
extern const uint FAST_LOG_PAGE_LIMIT;

/*
 * Number of blocks allowed to be in DFTL Cached Mapping Table.
 */
extern const uint CACHE_DFTL_LIMIT;

/*
 * FTL uses Block_manager class
 */
/// TODO Reset to const
/// extern const bool FTL_USE_BLOCKMANAGER;
// extern bool FTL_USE_BLOCKMANAGER;

/*
 * Overprovisioning factor for the xWF-based FTLs.
 */
/// TODO Reset to const
/// extern const double SPARE_FACTOR;
extern double SPARE_FACTOR;
extern double HOT_FRACTION;      // f
extern double HOT_REQUEST_RATIO; // r

/*
 * GC algorithm to use
 */
/// TODO Reset to const
/// extern const uint GC_ALGORITHM;
extern uint GC_ALGORITHM;

/*
 * Amount of choices D (for use in DChoices GCA)
 */
/// TODO Reset to const
extern uint DCHOICES_D;

/*
 * Wear leveler to use
 */
/// TODO Reset to const
extern uint WEAR_LEVELER;

/*
 * Amount of choices D for use in DChoices wear leveling
 */
/// TODO Reset to const
extern uint WLVL_BAN_D;

/*
 * Period between Ban wear leveling triggers at GC.
 */
extern ulong WLVL_BAN_TAU;

/*
 * Probability to trigger wear leveling at GC.
 */
extern double WLVL_ACTIVATION_PROBABILITY;

/*
 * Parallelism mode
 */
extern const uint PARALLELISM_MODE;

/* Virtual block size (as a multiple of the physical block size) */
extern const uint VIRTUAL_BLOCK_SIZE;

/* Virtual page size (as a multiple of the physical page size) */
extern const uint VIRTUAL_PAGE_SIZE;

extern const uint NUMBER_OF_ADDRESSABLE_BLOCKS;

/* RAISSDs: Number of physical SSDs */
extern const uint RAID_NUMBER_OF_PHYSICAL_SSDS;

/*
 * Memory area to support pages with data.
 */
extern void* page_data;
extern void* global_buffer;

/* Enumerations to clarify status integers in simulation
 * Do not use typedefs on enums for reader clarity */

/* Page states
 * 	empty   - page ready for writing (and contains no valid data)
 * 	valid   - page has been written to and contains valid data
 * 	invalid - page has been written to and does not contain valid data */
enum page_state {
	EMPTY,
	VALID,
	INVALID
};

/* Block states
 * 	free     - all pages in block are empty
 * 	active   - some pages in block are valid, others are empty or invalid
 * 	inactive - all pages in block are invalid */
enum block_state {
	FREE,
	ACTIVE,
	INACTIVE
};

/* I/O request event types
 * 	read  - read data from address
 * 	write - write data to address (page state set to valid)
 * 	erase - erase block at address (all pages in block are erased -
 * 	                                page states set to empty)
 * 	merge - move valid pages from block at address (page state set to
 * invalid)
 * 	           to free pages in block at merge_address */
enum event_type {
	READ,
	WRITE,
	ERASE,
	MERGE,
	TRIM
};

/* General return status
 * return status for simulator operations that only need to provide general
 * failure notifications */
enum status {
	FAILURE,
	SUCCESS
};

/* Address valid status
 * used for the valid field in the address class
 * example: if valid == BLOCK, then
 * 	the package, die, plane, and block fields are valid
 * 	the page field is not valid */
enum address_valid {
	NONE,
	PACKAGE,
	DIE,
	PLANE,
	BLOCK,
	PAGE
};

/*
 * Block type status
 * used for the garbage collector specify what pool
 * it should work with.
 * the block types are log, data and map (Directory map usually)
 */
enum block_type {
	LOG,
	DATA,
	LOG_SEQ
};

/*
 * Enumeration of the different FTL implementations.
 */
enum ftl_implementation {
	IMPL_PAGE,
	IMPL_BAST,
	IMPL_FAST,
	IMPL_DFTL,
	IMPL_BIMODAL,
	IMPL_SWF,
	IMPL_DWF,
	IMPL_HCWF,
	IMPL_COLD
};

/*
 * Enumeration of the different GC algorithms.
 */
enum gc_algorithm {
    GC_FIFO,
    GC_GREEDY,
    GC_RANDOM,
    GC_DCHOICES,
    GC_COSTBENEFIT,
    GC_COSTAGETIME,
    GC_DCHOICES_COSTBENEFIT,
    GC_DCHOICES_COSTAGETIME
};

/*
 * Enumeration of the different wear leveling schemes.
 */
enum wl_scheme {
	WL_NONE,
	WL_BAN,
	WL_BAN_PROB,
	WL_MAXVALID,
	WL_RANDOMSWAP,
	WL_HOTCOLDSWAP
};

/*
 * Enumeration of the different hot/cold identification techniques
 */
enum hc_ident {
	HCID_NONE,
	HCID_STATIC,
	HCID_ORACLE
};

#define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE 1

/* List classes up front for classes that have references to their "parent"
 * (e.g. a Package's parent is a Ssd).
 *
 * The order of definition below follows the order of this list to support
 * cases of agregation where the agregate class should be defined first.
 * Defining the agregate class first enables use of its non-default
 * constructors that accept args
 * (e.g. a Ssd contains a Controller, Ram, Bus, and Packages). */
class Address;
class Stats;
class Event;
class Channel;
class Bus;
class Page;
class Block;
class Plane;
class Die;
class Package;
class Garbage_collector;
class GCImpl_Random;
class GCImpl_DChoices;
class GCImpl_Greedy;
class Wear_leveler;
class WLvlImpl_Ban;
class WLvlImpl_HCSwitch;
class Block_manager;
class FtlParent;
class FtlImpl_Page;
class FtlImpl_Bast;
class FtlImpl_Fast;
class FtlImpl_DftlParent;
class FtlImpl_Dftl;
class FtlImpl_BDftl;
class FtlImpl_DWF;
class FtlImpl_HCWF;
class Ram;
class Controller;
class Ssd;

/* Class to manage physical addresses for the SSD.  It was designed to have
 * public members like a struct for quick access but also have checking,
 * printing, and assignment functionality.  An instance is created for each
 * physical address in the Event class. */
class Address
{
public:
	uint package;
	uint die;
	uint plane;
	uint block;
	uint page;
	ulong real_address;
	enum address_valid valid;
	Address(void);
	Address(const Address& address);
	Address(const Address* address);
	Address(uint package, uint die, uint plane, uint block, uint page,
	        enum address_valid valid);
	Address(uint address, enum address_valid valid);
	~Address();
	enum address_valid check_valid(uint ssd_size = SSD_SIZE,
	                               uint package_size = PACKAGE_SIZE,
	                               uint die_size = DIE_SIZE,
	                               uint plane_size = PLANE_SIZE,
	                               uint block_size = BLOCK_SIZE);
	enum address_valid compare(const Address& address) const;
	void print(FILE* stream = stdout);

	void operator+(int);
	void operator+(uint);
	Address& operator+=(const uint rhs);
	Address& operator=(const Address& rhs);
	bool same_block(const Address& rhs);

	void set_linear_address(ulong address, enum address_valid valid);
	void set_linear_address(ulong address);
	ulong get_linear_address() const;
};

/**
 * @brief Class that can determine whether a specific LPN is deemed as hot or
 * cold.
 */
class HotColdID
{
public:
	HotColdID() = default;
	virtual ~HotColdID() = default;
	/**
	 * @brief Checks whether the HCID has marked this LPN as hot.
	 * @param lpn Logical Page Number
	 * @return True if the LPN is hot
	 */
	virtual bool is_hot(const ulong lpn) const;
	/**
	 * @brief Lets the HCID know there is a request for this LPN.
	 * @param lpn Logical Page Number
	 * @return True if the LPN is hot
	 */
	virtual bool request_lpn(const ulong lpn);
};

class Oracle_HCID : public HotColdID
{
public:
	Oracle_HCID(const std::set<ulong>& hotSet);
	virtual ~Oracle_HCID();
	bool is_hot(const ulong lpn) const;

private:
	std::set<ulong> hotSet;
};

class Static_HCID : public HotColdID
{
public:
	Static_HCID(ulong maxLPN, double hotFraction = HOT_FRACTION);
	virtual ~Static_HCID();
	bool is_hot(const ulong lpn) const;

private:
	ulong maxLPN;
	double hotFraction;
	ulong numHotLPN;
	std::map<ulong, bool> hotMap;
};

class WARM_HCID : public HotColdID
{
public:
	WARM_HCID(const uint maxHotQueueLength, const uint maxCooldownLength);
	virtual ~WARM_HCID();

	bool is_hot(const ulong lpn) const;
	/**
	 * @brief Lets the HCID know there is a request for this lpn.
	 * @param lpn Logical Page Number
	 * @return True if the lpn is hot
	 */
	bool request_lpn(const ulong lpn);

private:
	uint maxNumHot;
	uint maxNumCooldown;
	std::deque<ulong> hotQueue;
	std::deque<ulong> cooldownWindow;
};

class Stats
{
public:
	// Flash Translation Layer
	ulong numFTLRead;
	ulong numFTLWrite;
	ulong numFTLErase;
	ulong numFTLTrim;

	// Garbage Collection
	ulong numGCRead;
	ulong numGCWrite;
	ulong numGCErase;

	// Wear-leveling
	long numWLRead;
	long numWLWrite;
	long numWLErase;

	// Log based FTL's
	long numLogMergeSwitch;
	long numLogMergePartial;
	long numLogMergeFull;

	// Page based FTL's
	long numPageBlockToPageConversion;

	// Cache based FTL's
	long numCacheHits;
	long numCacheFaults;

	// Possible SSD performance measures
	std::vector<double> PEfairness;
	std::vector<double> SSDendurance;
	std::vector<ulong> PEDistribution;

	// DWF
	std::vector<uint> WFEHotPagesDist;
	std::vector<uint> WFIHotPagesDist;
	std::vector<ulong> victimValidDist;

	// HCWF
	std::vector<ulong> hotValidPages;
	std::vector<uint> hotBlocks;
	std::vector<ulong> coldValidPages;
	std::vector<uint> coldBlocks;
	ulong firstColdErase;
    std::vector<ulong> hotVictim_ValidDist;
    std::vector<ulong> coldVictim_ValidDist;

    // Swap
    std::vector<ulong> swapCost;

	// Memory consumptions (Bytes)
	long numMemoryTranslation;
	long numMemoryCache;

	long numMemoryRead;
	long numMemoryWrite;

	// Advance statictics
	double translation_overhead() const;
	double variance_of_io() const;
	double cache_hit_ratio() const;

	// Advance currentPE and log some statistics
	uint get_currentPE() const;
	// void next_GC_invocation(const uint validPages, const uint hotValidPages,
	// const bool victimReplacesHWFOrWFE);
	void next_currentPE(const HotColdID* hcID = nullptr);

	// Constructors, maintainance, output, etc.
	Stats(void);

	void erase_block(
	    const uint validPagesOnVictim,
	    const bool isHotVictim = false); // Modify stats on block erasure
	void swap_blocks(const uint totalValidPages);
	void print_statistics();
	void reset_statistics();
	void write_statistics(FILE* stream);
	void write_statistics_csv(const std::string fileName, const uint runID,
	                          const std::string traceID = "");
	void write_header(FILE* stream);

private:
	uint currentPE;
	std::string create_filename(const std::string fileNameStart,
	                            const std::string fieldName, const uint runID,
	                            const std::string traceID = "");
	void write_csv(const std::string fileName, const uint value,
	               const uint begin = 0);
	void write_csv(const std::string fileName, const ulong value,
	               const uint begin = 0);
	void write_csv(const std::string fileName, const double value,
	               const uint begin = 0);
	void write_csv(const std::string fileName,
	               const std::vector<double>& vector, const uint begin = 0);
	void write_csv(const std::string fileName, const std::vector<uint>& vector,
	               const uint begin = 0);
	void write_csv(const std::string fileName, const std::vector<ulong>& vector,
	               const uint begin = 0);
	void reset();
};

/* Class to emulate a log block with page-level mapping. */
class LogPageBlock
{
public:
	LogPageBlock(void);
	~LogPageBlock(void);

	int* pages;
	long* aPages;
	Address address;
	int numPages;

	LogPageBlock* next;

	bool operator()(const ssd::LogPageBlock& lhs,
	                const ssd::LogPageBlock& rhs) const;
	bool operator()(const ssd::LogPageBlock*& lhs,
	                const ssd::LogPageBlock*& rhs) const;
};

/* Class to manage I/O requests as events for the SSD.  It was designed to keep
 * track of an I/O request by storing its type, addressing, and timing.  The
 * SSD class creates an instance for each I/O request it receives. */
class Event
{
public:
	Event(enum event_type type, ulong logical_address, uint size,
	      double start_time);
	~Event(void);
	void consolidate_metaevent(Event& list);
	ulong get_logical_address(void) const;
	const Address& get_address(void) const;
	const Address& get_merge_address(void) const;
	const Address& get_log_address(void) const;
	const Address& get_replace_address(void) const;
	uint get_size(void) const;
	enum event_type get_event_type(void) const;
	double get_start_time(void) const;
	double get_time_taken(void) const;
	double get_bus_wait_time(void) const;
	bool get_noop(void) const;
	bool is_hot() const;
	Event* get_next(void) const;
	void set_address(const Address& address);
	void set_merge_address(const Address& address);
	void set_log_address(const Address& address);
	void set_replace_address(const Address& address);
	void set_next(Event& next);
	void set_payload(void* payload);
	void set_event_type(const enum event_type& type);
	void set_noop(bool value);
	void set_hot(bool isHot);
	void* get_payload(void) const;
	double incr_bus_wait_time(double time);
	double incr_time_taken(double time_incr);
	void print(FILE* stream = stdout);

	void set_logical_address(const ulong newLPN); // ONLY use this if you MUST
private:
	double start_time;
	double time_taken;
	double bus_wait_time;
	enum event_type type;

	ulong logical_address;
	Address address;
	Address merge_address;
	Address log_address;
	Address replace_address;
	uint size;
	void* payload;
	Event* next;
	bool noop;
	bool hot;
};

struct IOEvent {
	event_type type;
	ulong lpn;
	uint size;
	IOEvent(const event_type type, const ulong lpn, const uint size)
		: type(type)
		, lpn(lpn)
		, size(size)
	{
	}
};

enum EVENT_READER_MODE {
	EVTRDR_SIMPLE,
	EVTRDR_BIOTRACER
};

class EventReader
{
public:
	EventReader(const std::string traceFileName, const ulong numEvents,
	            const EVENT_READER_MODE mode);
	EventReader(const std::string traceFileName, const ulong numEvents,
	            const EVENT_READER_MODE mode, const std::string oracleFileName);
	Event read_next_event();
	// std::set<ulong> read_accessed_lpns()
	//  const; // Scales with numEvents, avoid calling this frequently
	// std::set<ulong> read_accessed_lpns(
	//  const std::vector<IOEvent>& events) const;
	ulong find_max_lpn(const std::vector<IOEvent>& events) const;
	std::set<ulong> read_hot_lpns() const;
	std::vector<IOEvent> read_IO_events_from_trace(
	    const std::string& traceFile) const;
	std::vector<Event> read_events_from_trace(
	    const std::string& traceFile) const;
	std::string write_event(const Event& e) const;

private:
	void read_IO_event(const std::string& line,
	                   std::vector<IOEvent>& events) const;
	void read_IO_event_simple(const std::string& line,
	                          std::vector<IOEvent>& events) const;
	void read_IO_event_BIOtracer(const std::string& line,
	                             std::vector<IOEvent>& events) const;
	void read_event(const std::string& line, std::vector<Event>& events) const;
	void read_event_simple(const std::string& line,
	                       std::vector<Event>& events) const;
	void read_event_BIOtracer(const std::string& line,
	                          std::vector<Event>& events) const;
	std::string write_event_simple(const Event& e) const;
	std::string write_event_BIOtracer(const Event& e) const;
	bool read_next_oracle();

	EVENT_READER_MODE readMode;
	std::string traceFileName;
	std::string oracleFileName;
	ulong currentEvent;
	ulong numEvents;
	bool usingOracle;
	std::ifstream traceStream;
	std::ifstream oracleStream;
};

/* Single bus channel
 * Simulate multiple devices on 1 bus channel with variable bus transmission
 * durations for data and control delays with the Channel class.  Provide the
 * delay times to send a control signal or 1 page of data across the bus
 * channel, the bus table size for the maximum number channel transmissions that
 * can be queued, and the maximum number of devices that can connect to the bus.
 * To elaborate, the table size is the size of the channel scheduling table that
 * holds start and finish times of events that have not yet completed in order
 * to determine where the next event can be scheduled for bus utilization. */
class Channel
{
public:
	Channel(double ctrl_delay = BUS_CTRL_DELAY,
	        double data_delay = BUS_DATA_DELAY,
	        uint table_size = BUS_TABLE_SIZE,
	        uint max_connections = BUS_MAX_CONNECT);
	~Channel(void);
	enum status lock(double start_time, double duration, Event& event);
	enum status connect(void);
	enum status disconnect(void);
	double ready_time(void);

private:
	void unlock(double current_time);

	struct lock_times {
		double lock_time;
		double unlock_time;
	};

	static bool timings_sorter(lock_times const& lhs, lock_times const& rhs);
	std::vector<lock_times> timings;

	uint table_entries;
	uint selected_entry;
	uint num_connected;
	uint max_connections;
	double ctrl_delay;
	double data_delay;

	// Stores the highest unlock_time in the vector timings list.
	double ready_at;
};

/* Multi-channel bus comprised of Channel class objects
 * Simulates control and data delays by allowing variable channel lock
 * durations.  The sender (controller class) should specify the delay (control,
 * data, or both) for events (i.e. read = ctrl, ctrl+data; write = ctrl+data;
 * erase or merge = ctrl).  The hardware enable signals are implicitly
 * simulated by the sender locking the appropriate bus channel through the lock
 * method, then sending to multiple devices by calling the appropriate method
 * in the Package class. */
class Bus
{
public:
	Bus(uint num_channels = SSD_SIZE, double ctrl_delay = BUS_CTRL_DELAY,
	    double data_delay = BUS_DATA_DELAY, uint table_size = BUS_TABLE_SIZE,
	    uint max_connections = BUS_MAX_CONNECT);
	~Bus(void);
	enum status lock(uint channel, double start_time, double duration,
	                 Event& event);
	enum status connect(uint channel);
	enum status disconnect(uint channel);
	Channel& get_channel(uint channel);
	double ready_time(uint channel);

private:
	uint num_channels;
	Channel* const channels;
};

/* The page is the lowest level data storage unit that is the size unit of
 * requests (events).  Pages maintain their state as events modify them. */
class Page
{
public:
	Page(const Block& parent, double read_delay = PAGE_READ_DELAY,
	     double write_delay = PAGE_WRITE_DELAY);
	~Page(void);
	enum status _read(Event& event);
	enum status _write(Event& event);
	enum status _write(Event& event, const ulong lpn); // Increase time in
	// event, but use lpn;
	// useful for copying
	const Block& get_parent(void) const;
	enum page_state get_state(void) const;
	void set_state(enum page_state state);
	ulong get_logical_address() const;
	Page* get_pointer();

private:
	enum page_state state;
	const Block& parent;
	double read_delay;
	double write_delay;
	ulong lpn; // Associated logical page number, for easier lookup in FTLs.
};

/* The block is the data storage hardware unit where erases are implemented.
 * Blocks maintain wear statistics for the FTL. */
class Block
{
public:
	long physical_address;
	uint pages_invalid;
	Block(const Plane& parent, uint size = BLOCK_SIZE,
	      ulong erases_remaining = BLOCK_ERASES,
	      double erase_delay = BLOCK_ERASE_DELAY, long physical_address = 0);
	~Block(void);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status write(Event& event, uint& pageNr);
	enum status replace(Event& event);
	enum status _erase(Event& event);
	enum status _erase_and_copy(
	    Event& event, Address& copyBlock, Block* copyBlockPtr,
	    std::function<void(const ulong, const Address&)> modifyFTL,
	    std::function<void(const ulong, const uint pageNr)> modifyFTLPage);
	const Plane& get_parent(void) const;
	uint get_pages_valid(void) const;
	uint get_pages_invalid(void) const;
	uint get_pages_empty(void) const;
	enum block_state get_state(void) const;
	enum page_state get_state(uint page) const;
	enum page_state get_state(const Address& address) const;
	double get_last_erase_time(void) const;
    double get_last_page_invalidate_time(void) const;
    ///double get_modification_time(void) const;
    ulong get_erase_count(void) const;
	ulong get_erases_remaining(void) const;
	uint get_size(void) const;
	enum status get_next_page(Address& address) const;
	enum status get_next_page(uint& page) const;
    void invalidate_page(uint page, const double time);
	long get_physical_address(void) const;
	Page* get_page_pointer(const Address& addr);
	Block* get_pointer(void);
	block_type get_block_type(void) const;
	void set_block_type(block_type value);
	void set_block_hotness(const bool isHot);
	bool get_block_hotness() const;

	friend FtlParent;

private:
    static std::vector<ulong> _read_logical_addresses_and_data(Event& event,
	        const Block* block);
	static enum status _swap(
	    FtlParent* ftl, Event& event, const Address& block1,
	    const Address& block2,
	    std::function<void(const ulong lpn, const Address& newAddress)>
	    modifyFTL);

	uint size;
	Page* const data;
	const Plane& parent;
	uint pages_valid;
	enum block_state state;
    ulong max_erases;
	ulong erases_remaining;
	double last_erase_time;
    double last_page_invalidate_time;
	double erase_delay;
	double modification_time;

	// std::map<ulong,uint> block_map;
	bool isHot;

	block_type btype;
};

/* The plane is the data storage hardware unit that contains blocks.
 * Plane-level merges are implemented in the plane.  Planes maintain wear
 * statistics for the FTL. */
class Plane
{
public:
	Plane(const Die& parent, uint plane_size = PLANE_SIZE,
	      double reg_read_delay = PLANE_REG_READ_DELAY,
	      double reg_write_delay = PLANE_REG_WRITE_DELAY,
	      long physical_address = 0);
	~Plane(void);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status erase(Event& event);
	enum status replace(Event& event);
	enum status _merge(Event& event);
	const Die& get_parent(void) const;
	double get_last_erase_time(const Address& address) const;
	ulong get_erases_remaining(const Address& address) const;
	void get_least_worn(Address& address) const;
	uint get_size(void) const;
	enum page_state get_state(const Address& address) const;
	enum block_state get_block_state(const Address& address) const;
	void get_free_page(Address& address) const;
	ssd::uint get_num_free(const Address& address) const;
	ssd::uint get_num_valid(const Address& address) const;
	ssd::uint get_num_invalid(const Address& address) const;
	Page* get_page_pointer(const Address& addr);
	Block* get_block_pointer(const Address& address) const;
	Plane* get_pointer();
	// For specific blocks:
	uint get_pages_valid(const Address& address) const;
	uint get_pages_invalid(const Address& address) const;
	uint get_pages_erased(const Address& address) const;
	void set_block_hotness(const Address& address, const bool hotness);
	bool get_block_hotness(const Address& address) const;
	enum status get_next_page(Address& address) const;

private:
	void update_wear_stats(void);
	enum status get_next_page(void);
	uint size;
	Block* const data;
	const Die& parent;
	uint least_worn;
	ulong erases_remaining;
	double last_erase_time;
	double reg_read_delay;
	double reg_write_delay;
	Address next_page;
	uint free_blocks;
};

/* The die is the data storage hardware unit that contains planes and is a flash
 * chip.  Dies maintain wear statistics for the FTL. */
class Die
{
public:
	Die(const Package& parent, Channel& channel, uint die_size = DIE_SIZE,
	    long physical_address = 0);
	~Die(void);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status erase(Event& event);
	enum status replace(Event& event);
	enum status merge(Event& event);
	enum status _merge(Event& event);
	const Package& get_parent(void) const;
	double get_last_erase_time(const Address& address) const;
	ulong get_erases_remaining(const Address& address) const;
	void get_least_worn(Address& address) const;
	enum page_state get_state(const Address& address) const;
	enum block_state get_block_state(const Address& address) const;
	void get_free_page(Address& address) const;
	ssd::uint get_num_free(const Address& address) const;
	ssd::uint get_num_valid(const Address& address) const;
	ssd::uint get_num_invalid(const Address& address) const;
	Page* get_page_pointer(const Address& addr);
	Block* get_block_pointer(const Address& address) const;
	Plane* get_plane_pointer(const Address& address);
	// For specific blocks:
	uint get_pages_valid(const Address& address) const;
	uint get_pages_invalid(const Address& address) const;
	uint get_pages_erased(const Address& address) const;
	void set_block_hotness(const Address& address, const bool hotness);
	bool get_block_hotness(const Address& address) const;
	enum status get_next_page(Address& address) const;

private:
	void update_wear_stats(const Address& address);
	uint size;
	Plane* const data;
	const Package& parent;
	Channel& channel;
	uint least_worn;
	ulong erases_remaining;
	double last_erase_time;
};

/* The package is the highest level data storage hardware unit.  While the
 * package is a virtual component, events are passed through the package for
 * organizational reasons, including helping to simplify maintaining wear
 * statistics for the FTL. */
class Package
{
public:
	Package(const Ssd& parent, Channel& channel,
	        uint package_size = PACKAGE_SIZE, long physical_address = 0);
	~Package();
	enum status read(Event& event);
	enum status write(Event& event);
	enum status erase(Event& event);
	enum status replace(Event& event);
	enum status merge(Event& event);
	const Ssd& get_parent(void) const;
	double get_last_erase_time(const Address& address) const;
	ulong get_erases_remaining(const Address& address) const;
	void get_least_worn(Address& address) const;
	enum page_state get_state(const Address& address) const;
	enum block_state get_block_state(const Address& address) const;
	void get_free_page(Address& address) const;
	ssd::uint get_num_free(const Address& address) const;
	ssd::uint get_num_valid(const Address& address) const;
	ssd::uint get_num_invalid(const Address& address) const;
	Page* get_page_pointer(const Address& address);
	Block* get_block_pointer(const Address& address) const;
	Plane* get_plane_pointer(const Address& address);
	// For specific blocks:
	uint get_pages_valid(const Address& address) const;
	uint get_pages_invalid(const Address& address) const;
	uint get_pages_erased(const Address& address) const;
	void set_block_hotness(const Address& address, const bool hotness);
	bool get_block_hotness(const Address& address) const;
	enum status get_next_page(Address& address) const;

private:
	void update_wear_stats(const Address& address);
	uint size;
	Die* const data;
	const Ssd& parent;
	uint least_worn;
	ulong erases_remaining;
	double last_erase_time;
};

/* place-holder definitions for GC, WL, FTL, RAM, Controller
 * please make sure to keep this order when you replace with your definitions */
class Garbage_collector
{
public:
	Garbage_collector(FtlParent* FTL);
	virtual ~Garbage_collector(void);
    void collect(const Event& evt, Address& addr,
                         const std::vector<Address>& doNotPick);
    virtual void collect(const Event& evt, Address& addr,
                         const std::function<bool(const Address&)>& ignorePredicate);
protected:
	FtlParent* ftl;
};

class GCImpl_Random : public Garbage_collector
{
public:
	GCImpl_Random(FtlParent* FTL);
	~GCImpl_Random();
	void collect(const Event& evt, Address& addr,
                 const std::function<bool(const Address&)>& ignorePred);
};

class GCImpl_DChoices : public Garbage_collector
{
public:
	GCImpl_DChoices(FtlParent* FTL, const uint d = DCHOICES_D);
	~GCImpl_DChoices();
	void collect(const Event& evt, Address& addr,
                 const std::function<bool(const Address&)>& ignorePred);

private:
	uint d;
	ulong FIFOCounter;
	std::vector<ulong> choices;
};

class GCImpl_FIFO : public Garbage_collector
{
public:
	GCImpl_FIFO(FtlParent* FTL,
	            const Address startBlock = Address(0, 0, 0, 0, 0, PAGE));
	~GCImpl_FIFO();
	void collect(const Event& evt, Address& addr,
                 const std::function<bool(const Address&)>& ignorePred);

private:
	Address currentAddress;
};

class GCImpl_Greedy : public Garbage_collector
{
public:
	GCImpl_Greedy(FtlParent* FTL);
	~GCImpl_Greedy();
	void collect(const Event& evt, Address& addr,
                 const std::function<bool(const Address&)>& ignorePred);
};

class GCImpl_CostBenefit : public Garbage_collector
{
public:
    GCImpl_CostBenefit(FtlParent* FTL);
	~GCImpl_CostBenefit();
	void collect(const Event& evt, Address& addr,
                 const std::function<bool(const Address&)>& ignorePred);
};

class GCImpl_CostAgeTime : public Garbage_collector
{
public:
    GCImpl_CostAgeTime(FtlParent* FTL);
    ~GCImpl_CostAgeTime();
    void collect(const Event& evt, Address& addr,
                 const std::function<bool(const Address&)>& ignorePred);
};


class GCImpl_DChoices_CostBenefit : public Garbage_collector
{
public:
    GCImpl_DChoices_CostBenefit(FtlParent* FTL, const uint d = DCHOICES_D);
    ~GCImpl_DChoices_CostBenefit();
    void collect(const Event& evt, Address& addr,
                 const std::function<bool(const Address&)>& ignorePred);

private:
    uint d;
};

class GCImpl_DChoices_CostAgeTime : public Garbage_collector
{
public:
    GCImpl_DChoices_CostAgeTime(FtlParent* FTL, const uint d = DCHOICES_D);
    ~GCImpl_DChoices_CostAgeTime();
    void collect(const Event& evt, Address& addr,
                 const std::function<bool(const Address&)>& ignorePred);

private:
    uint d;
};

class Wear_leveler
{
public:
	Wear_leveler(FtlParent* FTL);
	virtual ~Wear_leveler(void);
	virtual enum status prewrite(Event& evt, Controller& controller,
	                             const std::vector<Address>& safeBlocks);
	virtual enum status suggest_WF(Address& WFSuggestion,
	                               const std::vector<Address>& doNotPick);

protected:
	FtlParent* FTL;
};

class WLvlImpl_Ban : public Wear_leveler
{
public:
	WLvlImpl_Ban(FtlParent* FTL, const ulong tau = WLVL_BAN_TAU);
	~WLvlImpl_Ban();
	enum status suggest_WF(Address& WFSuggestion,
	                       const std::vector<Address>& doNotPick);

private:
	ulong tau;
};

class WLvlImpl_Ban_Prob : public Wear_leveler
{
public:
	WLvlImpl_Ban_Prob(FtlParent* FTL,
	                  const double p = WLVL_ACTIVATION_PROBABILITY);
	~WLvlImpl_Ban_Prob();
	enum status suggest_WF(Address& WFSuggestion,
	                       const std::vector<Address>& doNotPick);

private:
	double p;
};

class WLvlImpl_RandomSwap : public Wear_leveler
{
public:
	WLvlImpl_RandomSwap(FtlParent* FTL,
	                    const double p = WLVL_ACTIVATION_PROBABILITY);
	~WLvlImpl_RandomSwap();
	enum status prewrite(Event& evt, Controller& controller,
	                     const std::vector<Address>& safeBlocks);
	enum status suggest_WF(Address& WFSuggestion,
	                       const std::vector<Address>& doNotPick);

private:
	double p;
};

class WLvlImpl_HotColdSwap : public Wear_leveler
{
public:
	WLvlImpl_HotColdSwap(FtlParent* FTL,
	                     const double p = WLVL_ACTIVATION_PROBABILITY);
	~WLvlImpl_HotColdSwap();
	enum status prewrite(Event& evt, Controller& controller,
	                     const std::vector<Address>& safeBlocks);
	enum status suggest_WF(Address& WFSuggestion,
	                       const std::vector<Address>& doNotPick);

private:
	double p;
};

class WLvlImpl_CleanMaxValidDCh : public Wear_leveler
{
public:
	WLvlImpl_CleanMaxValidDCh(FtlParent* FTL,
	                          const double p = WLVL_ACTIVATION_PROBABILITY,
	                          const uint d = DCHOICES_D);
	~WLvlImpl_CleanMaxValidDCh();
	enum status suggest_WF(Address& WFSuggestion,
	                       const std::vector<Address>& doNotPick);

private:
	double p;
	uint d;
};

class WLvlImpl_CleanMaxValid : public Wear_leveler
{
public:
	WLvlImpl_CleanMaxValid(FtlParent* FTL,
	                       const double p = WLVL_ACTIVATION_PROBABILITY);
	~WLvlImpl_CleanMaxValid();
	enum status suggest_WF(Address& WFSuggestion,
	                       const std::vector<Address>& doNotPick);

private:
	double p;
};

class Block_manager
{
public:
	Block_manager(FtlParent* ftl);
	~Block_manager(void);

	// Usual suspects
	Address get_free_block(Event& event);
	Address get_free_block(block_type btype, Event& event);
	void invalidate(Address address, block_type btype);
	void print_statistics();
	void insert_events(Event& event);
	void promote_block(block_type to_type);
	bool is_log_full();
	void erase_and_invalidate(Event& event, Address& address, block_type btype);
	int get_num_free_blocks();

	// Used to update GC on used pages in blocks.
	void update_block(Block* b);

	// Singleton
	static Block_manager* instance();
	static void instance_initialize(FtlParent* ftl);
	static Block_manager* inst;

	void cost_insert(Block* b);

	void print_cost_status();

private:
	void get_page_block(Address& address, Event& event);
	static bool block_comparitor_simple(Block const* x, Block const* y);

	FtlParent* ftl;

	ulong data_active;
	ulong log_active;
	ulong logseq_active;

	ulong max_log_blocks;
	ulong max_blocks;

	ulong max_map_pages;
	ulong map_space_capacity;

	// Cost/Benefit priority queue.
	typedef boost::multi_index_container<
	Block*, boost::multi_index::indexed_by<
	boost::multi_index::random_access<>,
	      boost::multi_index::ordered_non_unique<BOOST_MULTI_INDEX_MEMBER(
	          Block, uint, pages_invalid)>>>
	      active_set;

	typedef active_set::nth_index<0>::type ActiveBySeq;
	typedef active_set::nth_index<1>::type ActiveByCost;

	active_set active_cost;

	// Usual block lists
	std::vector<Block*> active_list;
	std::vector<Block*> free_list;
	std::vector<Block*> invalid_list;

	// Counter for returning the next free page.
	ulong directoryCurrentPage;
	// Address on the current cached page in SRAM.
	ulong directoryCachedPage;

	ulong simpleCurrentFree;

	// Counter for handling periodic sort of active_list
	uint num_insert_events;

	uint current_writing_block;

	bool inited;

	bool out_of_blocks;
};

/* Ftl class has some completed functions that get info from lower-level
 * hardware.  The other functions are in place as suggestions and can
 * be changed as you wish. */
class FtlParent
{
public:
	FtlParent(Controller& controller);

	virtual ~FtlParent();

	virtual void initialize(const ulong numUniqueLPN);
	virtual void initialize(const std::set<ulong>& uniqueLPNs);

	virtual enum status read(Event& event) = 0;
	virtual enum status write(Event& event) = 0;
	virtual enum status trim(Event& event) = 0;
	virtual void cleanup_block(Event& event, Block* block);

	virtual void print_ftl_statistics();

	friend class Block_manager;
	friend class Wear_leveler;
	friend class WLvlImpl_Ban;
	friend class WLvlImpl_Ban_Prob;
	friend class WLvlImpl_RandomSwap;
	friend class WLvlImpl_HotColdSwap;

	uint get_pages_valid(const Address& address) const;
	uint get_pages_invalid(const Address& address) const;
	uint get_pages_erased(const Address& address) const;
	ulong get_erases_remaining(const Address& address) const;
	void get_least_worn(Address& address) const;
	enum page_state get_state(const Address& address) const;
	enum block_state get_block_state(const Address& address) const;
	Address resolve_logical_address(unsigned int logicalAddress);
	Block* get_block(const Address& address) const;

protected:
	void set_block_hotness(const Address& address, const bool hotness);
	bool get_block_hotness(const Address& address) const;
	enum status get_next_page(Address& address) const;

	Controller& controller;
	Garbage_collector* garbage;
	Wear_leveler* wlvl;
	// For use by wear leveling schemes
    enum status swap(Event& event, const Address& block1, const Address& block2);
	virtual void modifyFTL(const ulong lpn, const Address& newAddress);
};

class FtlImpl_Page : public FtlParent
{
public:
	FtlImpl_Page(Controller& controller);
	~FtlImpl_Page();
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);

private:
	ulong currentPage;
	ulong numPagesActive;
	bool* trim_map;
	long* map;
};

class FtlImpl_Bast : public FtlParent
{
public:
	FtlImpl_Bast(Controller& controller);
	~FtlImpl_Bast();
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);

private:
	std::map<long, LogPageBlock*> log_map;

	long* data_list;

	void dispose_logblock(LogPageBlock* logBlock, long lba);
	void allocate_new_logblock(LogPageBlock* logBlock, long lba, Event& event);

	bool is_sequential(LogPageBlock* logBlock, long lba, Event& event);
	bool random_merge(LogPageBlock* logBlock, long lba, Event& event);

	void update_map_block(Event& event);

	void print_ftl_statistics();

	int addressShift;
	int addressSize;
};

class FtlImpl_Fast : public FtlParent
{
public:
	FtlImpl_Fast(Controller& controller);
	~FtlImpl_Fast();
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);

private:
	void initialize_log_pages();

	std::map<long, LogPageBlock*> log_map;

	long* data_list;
	bool* pin_list;

	bool write_to_log_block(Event& event, long logicalBlockAddress);

	void switch_sequential(Event& event);
	void merge_sequential(Event& event);
	bool random_merge(LogPageBlock* logBlock, Event& event);

	void update_map_block(Event& event);

	void print_ftl_statistics();

	long sequential_logicalblock_address;
	Address sequential_address;
	uint sequential_offset;

	uint log_page_next;
	LogPageBlock* log_pages;

	int addressShift;
	int addressSize;
};

class FtlImpl_DftlParent : public FtlParent
{
public:
	FtlImpl_DftlParent(Controller& controller);
	~FtlImpl_DftlParent();
	virtual enum status read(Event& event) = 0;
	virtual enum status write(Event& event) = 0;
	virtual enum status trim(Event& event) = 0;

protected:
	struct MPage {
		long vpn;
		long ppn;
		double create_ts;
		double modified_ts;
		double last_visited_time;
		bool cached;

		MPage(long vpn);
	};

	long int cmt;

	static double mpage_last_visited_time_compare(const MPage& mpage);

	typedef boost::multi_index_container<
	FtlImpl_DftlParent::MPage,
	                   boost::multi_index::indexed_by<
	                   // sort by MPage::operator<
	                   boost::multi_index::random_access<>,

	                   // Sort by last visited time
	                   boost::multi_index::ordered_non_unique<boost::multi_index::global_fun<
	                   const FtlImpl_DftlParent::MPage&, double,
	                   &FtlImpl_DftlParent::mpage_last_visited_time_compare>>>>
	                   trans_set;

	typedef trans_set::nth_index<0>::type MpageByID;
	typedef trans_set::nth_index<1>::type MpageByLastVisited;

	trans_set trans_map;
	long* reverse_trans_map;

	void consult_GTD(long dppn, Event& event);
	void reset_MPage(FtlImpl_DftlParent::MPage& mpage);

	void resolve_mapping(Event& event, bool isWrite);
	void update_translation_map(FtlImpl_DftlParent::MPage& mpage, long ppn);

	bool lookup_CMT(long dlpn, Event& event);

	long get_free_data_page(Event& event);
	long get_free_data_page(Event& event, bool insert_events);

	void evict_page_from_cache(Event& event);
	void evict_specific_page_from_cache(Event& event, long lba);

	// Mapping information
	int addressPerPage;
	int addressSize;
	uint totalCMTentries;

	// Current storage
	long currentDataPage;
	long currentTranslationPage;
};

class FtlImpl_Dftl : public FtlImpl_DftlParent
{
public:
	FtlImpl_Dftl(Controller& controller);
	~FtlImpl_Dftl();
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);
	void cleanup_block(Event& event, Block* block);
	void print_ftl_statistics();
};

class FtlImpl_BDftl : public FtlImpl_DftlParent
{
public:
	FtlImpl_BDftl(Controller& controller);
	~FtlImpl_BDftl();
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);
	void cleanup_block(Event& event, Block* block);

private:
	struct BPage {
		uint pbn;
		unsigned char nextPage;
		bool optimal;

		BPage();
	};

	BPage* block_map;
	bool* trim_map;

	std::queue<Block*> blockQueue;

	Block* inuseBlock;
	bool block_next_new();
	long get_free_biftl_page(Event& event);
	void print_ftl_statistics();
};

class FtlImpl_SWF
	: public FtlParent // Simulates the working of a page-mapped FTL
{
public:
	FtlImpl_SWF(Controller& controller);
	~FtlImpl_SWF();
    void initialize(const ulong maxLPN);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);

private:
    void check_ftl_integrity(const ulong numLPN);
    Address WF;   // 1 WF
	std::vector<Address> map;
};

class FtlImpl_DWF : public FtlParent
{
public:
	FtlImpl_DWF(Controller& controller, HotColdID* hcID);
	// FtlImpl_DWF(Controller &controller, const std::vector<Event> &events);
	~FtlImpl_DWF();
	void initialize(const ulong maxLPN);
	// void initialize(const std::set<ulong> &uniqueLPNs);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);

protected:
	void modifyFTL(const ulong lpn, const Address& address);

private:
	void modify_ftl_page(const ulong lpn, const uint newPage);
	void recompute_hotvalidpages(Address block);
    void check_ftl_integrity(const ulong lpn);
    void check_ftl_integrity();
    void check_valid_pages(const ulong numLPN);
    void check_hot_pages(Address blockAddr, Block* blockPtr, const uint hotPages);

	Address WFI; // Internal WF
	Address WFE; // External WF
	std::vector<Address> map;
	HotColdID* hcID;
	// std::vector<std::vector<std::vector<std::vector<uint>>>> hotValidPages;
};

class FtlImpl_HCWF : public FtlParent
{
public:
	FtlImpl_HCWF(Controller& controller, HotColdID* hcID);
	~FtlImpl_HCWF();
	virtual void initialize(const ulong maxLPN);
	// void initialize(const std::set<ulong> &uniqueLPNs);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);

protected:
	void modifyFTL(const ulong lpn, const Address& address);

private:
	// Correctness verification
	void check_valid_pages(const ulong numLPN);
	void check_block_hotness();
	void check_ftl_hotness_integrity();

	uint numHotBlocks;
	uint maxHotBlocks;
	Address CWF; // Cold WF
	Address HWF; // Hot WF
	// std::map<ulong, Address> map;
	std::vector<Address> map;
	HotColdID* hcID;
	// std::vector<std::vector<std::vector<std::vector<bool>>>> blockIsHot;
};

class FtlImpl_COLD : public FtlParent
{
public:
	FtlImpl_COLD(Controller& controller, HotColdID* hcID,
	             const uint d = DCHOICES_D);
	~FtlImpl_COLD();
	virtual void initialize(const ulong numUniqueLPN);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);

protected:
    void modifyFTL(const ulong lpn, const Address& address);

private:
	void gca_collect_COLD(const Event& event, Address& victimAddress,
	                      const bool replacingCWF,
	                      const std::vector<Address>& doNotPick);
	// Correctness verification
	void check_valid_pages(const ulong numLPN);
	void check_block_hotness();
	void check_ftl_hotness_integrity();

	uint d;
	ulong FIFOCounter;
	uint numHotBlocks;
	uint maxHotBlocks;
	Address CWF; // Cold WF
	Address HWF; // Hot WF
    std::vector<Address> map;
	HotColdID* hcID;
	// std::vector<std::vector<std::vector<std::vector<bool>>>> blockIsHot;
};

/* This is a basic implementation that only provides delay updates to events
 * based on a delay value multiplied by the size (number of pages) needed to
 * be written. */
class Ram
{
public:
	Ram(double read_delay = RAM_READ_DELAY,
	    double write_delay = RAM_WRITE_DELAY);
	~Ram(void);
	enum status read(Event& event);
	enum status write(Event& event);

private:
	double read_delay;
	double write_delay;
};

/* The controller accepts read/write requests through its event_arrive method
 * and consults the FTL regarding what to do by calling the FTL's read/write
 * methods.  The FTL returns an event list for the controller through its issue
 * method that the controller buffers in RAM and sends across the bus.  The
 * controller's issue method passes the events from the FTL to the SSD.
 *
 * The controller also provides an interface for the FTL to collect wear
 * information to perform wear-leveling.  */
class Controller
{
public:
	Controller(Ssd& parent, HotColdID* hcID);
	~Controller(void);
	void initialize(const ulong numLPN);
	void initialize(const std::set<ulong>& uniqueLPNs);
	enum status event_arrive(Event& event);
	friend class FtlParent;
	friend class FtlImpl_Page;
	friend class FtlImpl_Bast;
	friend class FtlImpl_Fast;
	friend class FtlImpl_DftlParent;
	friend class FtlImpl_Dftl;
	friend class FtlImpl_BDftl;
	friend class Block_manager;
	friend class FtlImpl_SWF;
	friend class FtlImpl_DWF;
	friend class FtlImpl_HCWF;
	friend class Garbage_collector;
	friend class GCImpl_Random;
	friend class GCImpl_DChoices;
	friend class GCImpl_Greedy;
	Stats stats;
	void print_ftl_statistics();
	const FtlParent& get_ftl(void) const;
	Page* get_page_pointer(const Address& address);
	Block* get_block(const Address& address) const;
	Plane* get_plane_pointer(const Address& address);
	uint get_pages_valid(const Address& address) const;
	uint get_pages_invalid(const Address& address) const;
	uint get_pages_erased(const Address& address) const;
	void set_block_hotness(const Address& address, const bool hotness);
	bool get_block_hotness(const Address& address) const;

private:
	enum status issue(Event& event_list);
	void translate_address(Address& address);
	ssd::ulong get_erases_remaining(const Address& address) const;
	void get_least_worn(Address& address) const;
	double get_last_erase_time(const Address& address) const;
	enum page_state get_state(const Address& address) const;
	enum block_state get_block_state(const Address& address) const;
	void get_free_page(Address& address) const;
	ssd::uint get_num_free(const Address& address) const;
	ssd::uint get_num_valid(const Address& address) const;
	ssd::uint get_num_invalid(const Address& address) const;
	enum status get_next_page(Address& address) const;

	Ssd& ssd;
	FtlParent* ftl;
};

/* The SSD is the single main object that will be created to simulate a real
 * SSD.  Creating a SSD causes all other objects in the SSD to be created.  The
 * event_arrive method is where events will arrive from DiskSim. */
class Ssd
{
public:
	Ssd(uint ssd_size = SSD_SIZE, HotColdID* hcID = nullptr);
	~Ssd(void);
	void initialize(const ulong maxLPN);
	/// TODO Pass EventReader instead of set of LPNs...
	// void initialize(const std::set<ulong> &uniqueLPNs);
	double event_arrive(enum event_type type, ulong logical_address, uint size,
	                    double start_time);
	double event_arrive(enum event_type type, ulong logical_address, uint size,
	                    double start_time, void* buffer);
	friend class Controller;
	void print_statistics();
	void reset_statistics();
	void write_statistics(FILE* stream);
	void write_statistics_csv(const std::string fileName, const uint runID,
	                          const std::string traceID = "");
	void write_header(FILE* stream);
	const Controller& get_controller(void) const;
	void* get_result_buffer();
	void print_ftl_statistics();
	double ready_at(void);
	Page* get_page_pointer(const Address& address);
	Block* get_block_pointer(const Address& address) const;
	Plane* get_plane_pointer(const Address& address);
	// For specific blocks:
	uint get_pages_valid(const Address& address) const;
	uint get_pages_invalid(const Address& address) const;
	uint get_pages_erased(const Address& address) const;
	void set_block_hotness(const Address& address, const bool hotness);
	bool get_block_hotness(const Address& address) const;

private:
	enum status read(Event& event);
	enum status write(Event& event);
	enum status erase(Event& event);
	enum status merge(Event& event);
	enum status replace(Event& event);
	enum status merge_replacement_block(Event& event);
	ulong get_erases_remaining(const Address& address) const;
	void update_wear_stats(const Address& address);
	void get_least_worn(Address& address) const;
	double get_last_erase_time(const Address& address) const;
	Package& get_data(void);
	enum page_state get_state(const Address& address) const;
	enum block_state get_block_state(const Address& address) const;
	void get_free_page(Address& address)
	const; /// TODO What is this in relation to get_next_page?
	ssd::uint get_num_free(const Address& address) const;
	ssd::uint get_num_valid(const Address& address) const;
	ssd::uint get_num_invalid(const Address& address) const;
	enum status get_next_page(Address& address) const;

	uint size;
	Controller controller;
	Ram ram;
	Bus bus;
	Package* const data;
	ulong erases_remaining;
	uint least_worn;
	double last_erase_time;
};

class RaidSsd
{
public:
	RaidSsd(uint ssd_size = SSD_SIZE);
	~RaidSsd(void);
	double event_arrive(enum event_type type, ulong logical_address, uint size,
	                    double start_time);
	double event_arrive(enum event_type type, ulong logical_address, uint size,
	                    double start_time, void* buffer);
	void* get_result_buffer();
	friend class Controller;
	void print_statistics();
	void reset_statistics();
	void write_statistics(FILE* stream);
	void write_header(FILE* stream);
	const Controller& get_controller(void) const;

	void print_ftl_statistics();

private:
	uint size;

	Ssd* Ssds;
};

/**
  Utilities and helper functions
*/

//std::string read_sim_parameters(uint argc, char* argv[]);
//std::string read_sim_trace_parameters(uint argc, char* argv[]);

/**
 * Checks whether a block is in a vector of blocks
 * @param[in] block Address of the block
 * @param[in] vector Vector of block addresses
 * @return True if block is in vector
 */
bool block_is_in_vector(const Address& block,
                        const std::vector<Address>& vector);
/**
 * Checks whether a block is in a vector of blocks
 * @param[in] block Pointer to the block
 * @param[in] vector Vector of block pointers
 * @return True if block pointer is in vector
 */
bool block_is_in_vector(const Block* block, const std::vector<Block*>& vector);

/**
 * Selects a block in a uniform random fashion.
 * @param[out] victimAddress Address that is selected
 */
void random_block(Address& victimAddress);

/**
 * Selects a block in a uniform random fashion.
 * @param[out] victimAddress Address that is selected
 * @param[in] doNotPick Addresses of blocks that are not to be selected.
 */
void random_block(Address& victimAddress,
                  const std::vector<Address>& doNotPick);

/**
 * Selects a block in a uniform random fashion.
 * @param[out] victimAddress Address that is selected
 * @param[in] ignorePredicate Block is not selected as one of the d choices if
 * this evaluates to true.
 */
void random_block(Address& victimAddress,
                  const std::function<bool(const Address&)>& ignorePredicate);

/**
 * Selects the block with minimal cost out of d randomly chosen blocks.
 * @tparam Return type of the cost function
 * @param[in] d Number of blocks (uniform) randomly selected.
 * @param[out] victimAddress Address that is selected
 * @param[in] costFunction Cost function to be minimised
 * @param[in] ignorePredicate Block is not selected as one of the d choices if
 * this evaluates to true.
 * @details Requires the < operator to be defined for type T
 */
// template <typename T>
// void d_choices_block(const unsigned int d, Address& victimAddress, const
// std::function<T(const Address&)>& costFunction,
//                     const std::function<bool(const Address&)>&
//                     ignorePredicate);
void d_choices_block(
    const unsigned int d, Address& victimAddress,
    const std::function<uint(const Address&)>& costFunction,
    const std::function<bool(const Address&)>& ignorePredicate);

/**
 * Exhaustively selects the block with minimal cost out of all blocks on the
 * SSD.
 * @tparam Return type of the cost function
 * @param[in] d Number of blocks (uniform) randomly selected.
 * @param[out] victimAddress Address that is selected
 * @param[in] costFunction Cost function to be minimised
 * @param[in] ignorePredicate Block is not selected if this evaluates to true.
 * @details Requires the < operator to be defined for type T
 */
// template <typename T>
// void greedy_block(Address& victimAddress, const std::function<T(const
// Address&)>& costFunction,
//                  const std::function<bool(const Address&)>& ignorePredicate);
void greedy_block(Address& victimAddress,
                  const std::function<uint(const Address&)>& costFunction,
                  const std::function<bool(const Address&)>& ignorePredicate);

void greedy_block(Address& victimAddress,
                  const std::function<double(const Address&)>& costFunction,
                  const std::function<bool(const Address&)>& ignorePredicate);

} /* end namespace ssd */

#endif