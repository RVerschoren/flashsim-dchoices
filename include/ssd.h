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
#ifndef NOT_USE_BLOCKMGR
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>*/
#endif
#include <algorithm>
#include <cstdio>
#include <deque>
#include <fstream>
#include <functional>
#include <map> /// TODO Replace with unordered_map?
#include <queue>
#include <random>
#include <set> /// TODO Replace with unordered_set?
#include <string>
#include <unordered_map>
#include <utility>
#include <valarray>
#include <vector>

#ifndef _SSD_H
#define _SSD_H
namespace ssd {

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
void load_config(const char* const config_name = "ssd.conf");
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
extern double HOT_FRACTION;				 // f
extern double HOT_REQUEST_RATIO;		 // r
extern double HOT_SPARE_FACTOR_FRACTION; // p
extern double HCID_FALSE_POS_RATIO;		 // False positives in H/C Identification
extern double HCID_FALSE_NEG_RATIO;		 // False negatives in H/C Identification
extern ulong MAX_PE;
extern ulong
	MAXPE_THRESHOLD; // Threshold for number of blocks that must have a certain PE cycle count to advance to the next
extern double HCSWAPWF_PROBABILITY;
extern uint MAX_ERASE_DIFF;

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
extern uint DCHOICES_DH;
extern uint DCHOICES_DC;
extern uint HCSWAPWF_D;

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
enum page_state
{
	EMPTY,
	VALID,
	INVALID
};

/* Block states
 * 	free     - all pages in block are empty
 * 	active   - some pages in block are valid, others are empty or invalid
 * 	inactive - all pages in block are invalid */
enum block_state
{
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
enum event_type
{
	READ,
	WRITE,
	ERASE,
	MERGE,
	TRIM
};

/* General return status
 * return status for simulator operations that only need to provide general
 * failure notifications */
enum status
{
	FAILURE,
	SUCCESS
};

/* Address valid status
 * used for the valid field in the address class
 * example: if valid == BLOCK, then
 * 	the package, die, plane, and block fields are valid
 * 	the page field is not valid */
enum address_valid
{
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
enum block_type
{
	LOG,
	DATA,
	LOG_SEQ
};

/*
 * Enumeration of the different FTL implementations.
 */
enum ftl_implementation
{
	IMPL_PAGE,
	IMPL_BAST,
	IMPL_FAST,
	IMPL_DFTL,
	IMPL_BIMODAL,
    IMPL_SWF,
    IMPL_SWFWDIFF,
	IMPL_DWF,
    IMPL_DWFWDIFF,
	IMPL_HCWF,
	IMPL_HCWF_DHC,
	IMPL_HCWF_FALSE,
	IMPL_HCWF_FIXED_FALSE,
    IMPL_HCWFWDIFF,
	IMPL_COLD,
	IMPL_COLD_DHC,
	IMPL_HCSWAPWF,
	IMPL_HCWFPLUSSWAP,
    IMPL_HCSWAPWF_ERASETIE,
    IMPL_HCSWAPWF_ERASE,
    IMPL_HCSWAPWF_ERASE_VALIDTIE,
    IMPL_HCSWAPWF_HOTCOLDTIE,
	IMPL_STAT,
	IMPL_DSWAPWF
};

/*
 * Enumeration of the different GC algorithms.
 */
enum gc_algorithm
{
    GC_FIFO,
    GC_GREEDY,
	GC_RANDOM,
	GC_DCHOICES,
	GC_DCH_HOTCOLD,
	GC_COSTBENEFIT,
	GC_COSTAGETIME,
	GC_DCHOICES_COSTBENEFIT,
    GC_DCHOICES_COSTAGETIME,
    GC_DCHOICES_ERASETIE,
    GC_DCHOICES_ERASE,
    GC_DCHOICES_ERASE_VALIDTIE

};

/*
 * Enumeration of the different wear leveling schemes.
 */
enum wl_scheme
{
	WL_NONE,
	WL_BAN,
	WL_BAN_PROB,
	WL_MAXVALID,
	WL_RANDOMSWAP,
	WL_HOTCOLDSWAP,
	WL_HOTCOLDGCSWAP,
    WL_HOTCOLDGCSWAP_ERASE,
	WL_HOTCOLDSWAP_RELATIVE // Upper bound on relative increase of WA
};

/*
 * Enumeration of the different hot/cold identification techniques
 */
enum hc_ident
{
	HCID_NONE,
	HCID_STATIC,
	HCID_STATIC_FALSE,
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
class Block_manager;
class FtlParent;
/*class FtlImpl_Page;
class FtlImpl_Bast;
class FtlImpl_Fast;
class FtlImpl_DftlParent;
class FtlImpl_Dftl;
class FtlImpl_BDftl;*/
class FtlImpl_HCWFFixedFalse;
class FtlImpl_HCWFWDiff;
class FtlImpl_DWFWDiff;
class FtlImpl_DSwapWF;
class FtlImpl_HCSwapWF;
class FtlImpl_HCWFPlusSwap;
class FtlImpl_HCSwapWF_EraseTie;
class FtlImpl_HCSwapWF_Erase;
class FtlImpl_HCSwapWF_Erase_ValidTie;
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
	Address(uint package, uint die, uint plane, uint block, uint page, enum address_valid valid);
	Address(uint address, enum address_valid valid);
	~Address();
	enum address_valid check_valid(uint ssd_size = SSD_SIZE, uint package_size = PACKAGE_SIZE, uint die_size = DIE_SIZE,
								   uint plane_size = PLANE_SIZE, uint block_size = BLOCK_SIZE);
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
using Addresses = std::vector<Address>;

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
};

/**
 * @brief The Static_False_HCID class
 * @deprecated
 */
class Static_False_HCID : public HotColdID
{
	public:
	Static_False_HCID(ulong maxLPN, double false_pos_prob, double false_neg_prob, double hotFraction = HOT_FRACTION);
	virtual ~Static_False_HCID();
	bool is_hot(const ulong lpn) const;

	private:
	ulong maxLPN;
	double fp;
	double fn;
	double hotFraction;
	ulong numHotLPN;
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
	std::vector<ulong> numHotVictimBlocks;
	std::vector<ulong> numColdVictimBlocks;
	uint numHotBlocks, numColdBlocks;
	ulong firstColdErase;
	std::vector<ulong> hotVictim_ValidDist;
	std::vector<ulong> coldVictim_ValidDist;
	std::vector<ulong> hotVictim_HotValidDist;
	std::vector<ulong> coldVictim_HotValidDist;
	std::vector<ulong> hotGCChainLength;
	std::vector<ulong> coldGCChainLength;
	ulong numExternalGCInvocations;

	// Swap
	std::vector<ulong> WLSwapCost;
	std::vector<ulong> FTLSwapCost;
	std::vector<ulong> numWLSwaps;
	ulong nWLSwaps;
	std::vector<ulong> numFTLSwaps;
	ulong nFTLSwaps;

	// Memory consumptions (Bytes)
	long numMemoryTranslation;
	long numMemoryCache;

	long numMemoryRead;
	long numMemoryWrite;

	// Advance statictics
	double translation_overhead() const;
	double variance_of_io() const;
	double cache_hit_ratio() const;
	double getWA() const;

	// Advance currentPE and log some statistics
	uint get_currentPE() const;
	// void next_GC_invocation(const uint validPages, const uint hotValidPages,
	// const bool victimReplacesHWFOrWFE);

	// Constructors, maintainance, output, etc.
	Stats(void);

	void erase_block(const uint validPagesOnVictim, const ulong remainingErasesOfVictim, const bool isHotVictim = false,
					 const uint hotValidPagesOnVictim = 0,
					 const ulong numHotBlocks = 0); // Modify stats on block erasure
	void swap_blocks(const uint totalValidPages, uint numBlocksErased = 2);
	void print_statistics();
	void reset_statistics();
	void write_statistics(FILE* stream);
	void write_statistics_csv(const std::string& fileName, const uint runID, const Ssd& ssd);
	void write_statistics_csv(const std::string& fileName, const uint runID);
	void write_header(FILE* stream);

	private:
	uint currentPE;
	uint numCurrentPE;
	ulong numHotVictims;
	ulong numColdVictims;
	uint thresholdCurrentPE;
	std::vector<ulong> numBlocksPEcycles;

	std::string create_filename(const std::string& fileNameStart, const std::string& fieldName, const uint runID);
	void write_csv(const std::string& fileName, const uint value, const uint begin = 0);
	void write_csv(const std::string& fileName, const ulong value, const uint begin = 0);
	void write_csv(const std::string& fileName, const double value, const uint begin = 0);
	void write_csv(const std::string& fileName, const std::vector<double>& vector, const uint begin = 0);
	void write_csv(const std::string& fileName, const std::vector<uint>& vector, const uint begin = 0);
	void write_csv(const std::string& fileName, const std::vector<ulong>& vector, const uint begin = 0);
	void reset();

	void next_currentPE(const HotColdID* hcID = nullptr);
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

	bool operator()(const ssd::LogPageBlock& lhs, const ssd::LogPageBlock& rhs) const;
	bool operator()(const ssd::LogPageBlock*& lhs, const ssd::LogPageBlock*& rhs) const;
};

/* Class to manage I/O requests as events for the SSD.  It was designed to keep
 * track of an I/O request by storing its type, addressing, and timing.  The
 * SSD class creates an instance for each I/O request it receives. */
class Event
{
	public:
	Event(enum event_type type, ulong logical_address, uint size, double start_time);
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

struct IOEvent
{
	event_type type;
	ulong lpn;
	uint size;
	IOEvent(const event_type type, const ulong lpn, const uint size) : type(type), lpn(lpn), size(size) {}
};

enum EVENT_READER_MODE
{
	EVTRDR_SIMPLE,
	EVTRDR_BIOTRACER
};

class EventReader
{
	public:
	EventReader(const std::string traceFileName, const ulong numEvents, const EVENT_READER_MODE mode);
	EventReader(const std::string traceFileName, const ulong numEvents, const EVENT_READER_MODE mode,
				const std::string oracleFileName);
	Event read_next_event();
	// std::set<ulong> read_accessed_lpns()
	//  const; // Scales with numEvents, avoid calling this frequently
	// std::set<ulong> read_accessed_lpns(
	//  const std::vector<IOEvent>& events) const;
	ulong find_max_lpn(const std::vector<IOEvent>& events) const;
	std::set<ulong> read_hot_lpns() const;
	std::vector<IOEvent> read_IO_events_from_trace(const std::string& traceFile) const;
	std::vector<Event> read_events_from_trace(const std::string& traceFile) const;
	std::string write_event(const Event& e) const;

	private:
	void read_IO_event(const std::string& line, std::vector<IOEvent>& events) const;
	void read_IO_event_simple(const std::string& line, std::vector<IOEvent>& events) const;
	void read_IO_event_BIOtracer(const std::string& line, std::vector<IOEvent>& events) const;
	void read_event(const std::string& line, std::vector<Event>& events) const;
	void read_event_simple(const std::string& line, std::vector<Event>& events) const;
	void read_event_BIOtracer(const std::string& line, std::vector<Event>& events) const;
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
	Channel(double ctrl_delay = BUS_CTRL_DELAY, double data_delay = BUS_DATA_DELAY, uint table_size = BUS_TABLE_SIZE,
			uint max_connections = BUS_MAX_CONNECT);
	~Channel(void);
	enum status lock(double start_time, double duration, Event& event);
	enum status connect(void);
	enum status disconnect(void);
	double ready_time(void);

	private:
	void unlock(double current_time);

	struct lock_times
	{
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
	Bus(uint num_channels = SSD_SIZE, double ctrl_delay = BUS_CTRL_DELAY, double data_delay = BUS_DATA_DELAY,
		uint table_size = BUS_TABLE_SIZE, uint max_connections = BUS_MAX_CONNECT);
	~Bus(void);
	enum status lock(uint channel, double start_time, double duration, Event& event);
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
	Page(const Block& parent, double read_delay = PAGE_READ_DELAY, double write_delay = PAGE_WRITE_DELAY);
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
	Block(const Plane& parent, uint size = BLOCK_SIZE, ulong erases_remaining = BLOCK_ERASES,
		  double erase_delay = BLOCK_ERASE_DELAY, long physical_address = 0);
	~Block(void);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status write(Event& event, uint& pageNr);
	enum status replace(Event& event);
	enum status _erase(Event& event);
    enum status _erase_and_copy(Event& event, const Address& copyBlock, Block* copyBlockPtr,
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
	/// double get_modification_time(void) const;
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
	friend FtlImpl_HCWFFixedFalse;
    friend FtlImpl_HCWFWDiff;
    friend FtlImpl_DWFWDiff;
	friend FtlImpl_DSwapWF;
	friend FtlImpl_HCSwapWF;
	friend FtlImpl_HCWFPlusSwap;
    friend FtlImpl_HCSwapWF_EraseTie;
    friend FtlImpl_HCSwapWF_Erase;
    friend FtlImpl_HCSwapWF_Erase_ValidTie;

    friend void d_choices_block_same_plane_min_valid_pages(Controller& ctrl, const unsigned int d,
                                                           Address& victimAddress,
                                                           const std::function<bool(const Address&)>& ignored);
    friend void greedy_block_same_plane_min_valid_pages(Controller& ctrl, Address& victimAddress,
                                                        const std::function<bool(const Address&)>& ignored);
    friend void d_choices_block_same_plane_min_valid_pages_tie_min_erase(
        Controller& ctrl, const unsigned int d, Address& victimAddress,
        const std::function<bool(const Address&)>& ignored);
    friend void d_choices_block_same_plane_min_valid_pages_tie_max_erase(
        Controller& ctrl, const unsigned int d, Address& victimAddress,
        const std::function<bool(const Address&)>& ignored);
    friend void greedy_block_same_plane_min_valid_pages_tie_min_erase(
        Controller& ctrl, Address& victimAddress, const std::function<bool(const Address&)>& ignored);

    friend void greedy_block_same_plane_min_valid_pages_tie_max_erase(
        Controller& ctrl, Address& victimAddress, const std::function<bool(const Address&)>& ignored);

    friend void d_choices_block_same_plane_min_erase(Controller& ctrl, const unsigned int d, Address& victimAddress,
                                                     const std::function<bool(const Address&)>& ignored);
    friend void greedy_block_same_plane_min_erase(Controller& ctrl, Address& victimAddress,
                                                  const std::function<bool(const Address&)>& ignored);
    friend void d_choices_block_same_plane_min_erase_tie_valid(Controller& ctrl, const unsigned int d,
                                                               Address& victimAddress,
                                                               const std::function<bool(const Address&)>& ignored);
    friend void greedy_block_same_plane_min_erase_tie_valid(Controller& ctrl, Address& victimAddress,
                                                            const std::function<bool(const Address&)>& ignored);

	private:
	std::vector<ulong> _read_logical_addresses_and_data(Event& event) const;
	static enum status _swap(FtlParent* ftl, Event& event, const Address& block1, const Address& block2,
							 std::function<void(const ulong lpn, const Address& newAddress)> modifyFTL);

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
	Plane(const Die& parent, uint plane_size = PLANE_SIZE, double reg_read_delay = PLANE_REG_READ_DELAY,
		  double reg_write_delay = PLANE_REG_WRITE_DELAY, long physical_address = 0);
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
    uint get_block_pages_valid(const Address& address) const;
    uint get_block_pages_invalid(const Address& address) const;
    uint get_block_pages_erased(const Address& address) const;
	void set_block_hotness(const Address& address, const bool hotness);
	bool get_block_hotness(const Address& address) const;
	enum status get_next_page(Address& address) const;
    ulong get_block_erase_count(const Address& address) const;

    friend void d_choices_block_same_plane_min_valid_pages(Controller& ctrl, const unsigned int d,
                                                           Address& victimAddress,
                                                           const std::function<bool(const Address&)>& ignored);

    friend void d_choices_block_same_plane_min_erase(Controller& ctrl, const unsigned int d, Address& victimAddress,
                                                     const std::function<bool(const Address&)>& ignored);
    friend void d_choices_block_same_plane_min_valid_pages_tie_min_erase(
        Controller& ctrl, const unsigned int d, Address& victimAddress,
        const std::function<bool(const Address&)>& ignored);
    friend void d_choices_block_same_plane_min_valid_pages_tie_max_erase(
        Controller& ctrl, const unsigned int d, Address& victimAddress,
        const std::function<bool(const Address&)>& ignored);

    friend void d_choices_block_same_plane_min_erase_tie_valid(Controller& ctrl, const unsigned int d,
                                                               Address& victimAddress,
                                                               const std::function<bool(const Address&)>& ignored);
    friend void greedy_block_same_plane_min_valid_pages_tie_min_erase(
        Controller& ctrl, Address& victimAddress, const std::function<bool(const Address&)>& ignored);

    friend void greedy_block_same_plane_min_valid_pages_tie_max_erase(
        Controller& ctrl, Address& victimAddress, const std::function<bool(const Address&)>& ignored);

    friend void greedy_block_same_plane_min_valid_pages(Controller& ctrl, Address& victimAddress,
                                                        const std::function<bool(const Address&)>& ignored);
    friend void greedy_block_same_plane_min_valid_pages_tie_erase(Controller& ctrl, Address& victimAddress,
                                                                  const std::function<bool(const Address&)>& ignored,
                                                                  bool preferLeastErasures);
    friend void greedy_block_same_plane_min_erase(Controller& ctrl, Address& victimAddress,
                                                  const std::function<bool(const Address&)>& ignored);
    friend void greedy_block_same_plane_min_erase_tie_valid(Controller& ctrl, Address& victimAddress,
                                                            const std::function<bool(const Address&)>& ignored);

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
	Die(const Package& parent, Channel& channel, uint die_size = DIE_SIZE, long physical_address = 0);
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
    uint get_block_pages_valid(const Address& address) const;
    uint get_block_pages_invalid(const Address& address) const;
    uint get_block_pages_erased(const Address& address) const;
	void set_block_hotness(const Address& address, const bool hotness);
	bool get_block_hotness(const Address& address) const;
	enum status get_next_page(Address& address) const;
    ulong get_block_erase_count(const Address& address) const;

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
	Package(const Ssd& parent, Channel& channel, uint package_size = PACKAGE_SIZE, long physical_address = 0);
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
    ulong get_block_erase_count(const Address& address) const;

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
    void collect(const Event& evt, Address& addr, const Addresses& doNotPick, bool replacingHotBlock);
    void collect(const Event& evt, Address& addr, const Addresses& doNotPick,
                 const std::pair<Address, Address>& blockAddressRange, bool replacingHotBlock);
    virtual void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePredicate,
                         bool replacingHotBlock);

	protected:
	FtlParent* ftl;
};

class GCImpl_Random : public Garbage_collector
{
	public:
	GCImpl_Random(FtlParent* FTL);
	~GCImpl_Random();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);
};

class GCImpl_DChoices : public Garbage_collector
{
	public:
	GCImpl_DChoices(FtlParent* FTL, const uint d = DCHOICES_D);
	~GCImpl_DChoices();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);

	private:
    uint d;
	std::vector<ulong> choices;
};

class GCImpl_DCh_HotCold : public Garbage_collector
{
    public:
    GCImpl_DCh_HotCold(FtlParent* FTL, const uint d = DCHOICES_D);
    ~GCImpl_DCh_HotCold();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);

    private:
    uint d;
    std::vector<ulong> choices;
};

class GCImpl_DCh_TieErasures : public Garbage_collector
{
    public:
    GCImpl_DCh_TieErasures(FtlParent* FTL, const uint d = DCHOICES_D);
    ~GCImpl_DCh_TieErasures();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);

    private:
    uint d;
    std::vector<ulong> choices;
};

class GCImpl_DCh_Erase : public Garbage_collector
{
    public:
    GCImpl_DCh_Erase(FtlParent* FTL, const uint d = DCHOICES_D);
    ~GCImpl_DCh_Erase();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);

    private:
    uint d;
    std::vector<ulong> choices;
};

class GCImpl_DCh_Erase_TieValid : public Garbage_collector
{
    public:
    GCImpl_DCh_Erase_TieValid(FtlParent* FTL, const uint d = DCHOICES_D);
    ~GCImpl_DCh_Erase_TieValid();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);

    private:
    uint d;
    std::vector<ulong> choices;
};

class GCImpl_FIFO : public Garbage_collector
{
	public:
	GCImpl_FIFO(FtlParent* FTL, const Address startBlock = Address(0, 0, 0, 0, 0, PAGE));
	~GCImpl_FIFO();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);

	private:
	Address currentAddress;
};

class GCImpl_Greedy : public Garbage_collector
{
	public:
	GCImpl_Greedy(FtlParent* FTL);
	~GCImpl_Greedy();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);
};

class GCImpl_CostBenefit : public Garbage_collector
{
	public:
	GCImpl_CostBenefit(FtlParent* FTL);
	~GCImpl_CostBenefit();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);
};

class GCImpl_CostAgeTime : public Garbage_collector
{
	public:
	GCImpl_CostAgeTime(FtlParent* FTL);
	~GCImpl_CostAgeTime();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);
};

class GCImpl_DChoices_CostBenefit : public Garbage_collector
{
	public:
	GCImpl_DChoices_CostBenefit(FtlParent* FTL, const uint d = DCHOICES_D);
	~GCImpl_DChoices_CostBenefit();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);

	private:
	uint d;
};

class GCImpl_DChoices_CostAgeTime : public Garbage_collector
{
	public:
	GCImpl_DChoices_CostAgeTime(FtlParent* FTL, const uint d = DCHOICES_D);
	~GCImpl_DChoices_CostAgeTime();
    void collect(const Event& evt, Address& addr, const std::function<bool(const Address&)>& ignorePred,
                 bool replacingHotBlock);

	private:
	uint d;
};

class Wear_leveler
{
	public:
	Wear_leveler(FtlParent* FTL);
	virtual ~Wear_leveler(void);
    virtual enum status prewrite(Event& evt, Controller& controller, const Addresses& safeBlocks);
	virtual enum status suggest_WF(Event& evt, Address& WFSuggestion, Controller& controller,
                                   const Addresses& doNotPick);

	protected:
	FtlParent* FTL;
};

class WLvlImpl_Ban : public Wear_leveler
{
	public:
	WLvlImpl_Ban(FtlParent* FTL, const ulong tau = WLVL_BAN_TAU);
	~WLvlImpl_Ban();
    enum status suggest_WF(Event& evt, Address& WFSuggestion, Controller& controller, const Addresses& doNotPick);

	private:
	ulong tau;
};

class WLvlImpl_Ban_Prob : public Wear_leveler
{
	public:
	WLvlImpl_Ban_Prob(FtlParent* FTL, const double p = WLVL_ACTIVATION_PROBABILITY, const uint d = WLVL_BAN_D);
	~WLvlImpl_Ban_Prob();
    enum status suggest_WF(Event& evt, Address& WFSuggestion, Controller& controller, const Addresses& doNotPick);

	private:
	double p;
	uint d;
};

class WLvlImpl_RandomSwap : public Wear_leveler
{
	public:
	WLvlImpl_RandomSwap(FtlParent* FTL, const double p = WLVL_ACTIVATION_PROBABILITY);
	~WLvlImpl_RandomSwap();
    enum status prewrite(Event& evt, Controller& controller, const Addresses& safeBlocks);
    enum status suggest_WF(Event& evt, Address& WFSuggestion, Controller& controller, const Addresses& doNotPick);

	private:
	double p;
};

class WLvlImpl_HotColdSwap : public Wear_leveler
{
	public:
	WLvlImpl_HotColdSwap(FtlParent* FTL, const double p = WLVL_ACTIVATION_PROBABILITY);
	~WLvlImpl_HotColdSwap();
    enum status prewrite(Event& evt, Controller& controller, const Addresses& safeBlocks);
    enum status suggest_WF(Event& evt, Address& WFSuggestion, Controller& controller, const Addresses& doNotPick);

	private:
	double p;
};

class WLvlImpl_HotColdGCSwap : public Wear_leveler
{
	public:
	WLvlImpl_HotColdGCSwap(FtlParent* FTL, const double p = WLVL_ACTIVATION_PROBABILITY);
	~WLvlImpl_HotColdGCSwap();
    enum status suggest_WF(Event& evt, Address& WFSuggestion, Controller& controller, const Addresses& doNotPick);

	private:
	double p;
};

class WLvlImpl_HotColdGCSwap_Erase : public Wear_leveler
{
    public:
    WLvlImpl_HotColdGCSwap_Erase(FtlParent* FTL, const double p = WLVL_ACTIVATION_PROBABILITY);
    ~WLvlImpl_HotColdGCSwap_Erase();
    enum status suggest_WF(Event& evt, Address& WFSuggestion, Controller& controller, const Addresses& doNotPick);

    private:
    double p;
};

class WLvlImpl_CleanMaxValidDCh : public Wear_leveler
{
	public:
	WLvlImpl_CleanMaxValidDCh(FtlParent* FTL, const double p = WLVL_ACTIVATION_PROBABILITY, const uint d = DCHOICES_D);
	~WLvlImpl_CleanMaxValidDCh();
    enum status suggest_WF(Event& evt, Address& WFSuggestion, Controller& controller, const Addresses& doNotPick);

	private:
	double p;
	uint d;
};

class WLvlImpl_CleanMaxValid : public Wear_leveler
{
	public:
	WLvlImpl_CleanMaxValid(FtlParent* FTL, const double p = WLVL_ACTIVATION_PROBABILITY);
	~WLvlImpl_CleanMaxValid();
    enum status suggest_WF(Event& evt, Address& WFSuggestion, Controller& controller, const Addresses& doNotPick);

	private:
	double p;
};

#ifndef NOT_USE_BLOCKMGR
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
					boost::multi_index::ordered_non_unique<BOOST_MULTI_INDEX_MEMBER(Block, uint, pages_invalid)>>>
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
#endif

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
	friend class WLvlImpl_HotColdGCSwap;
    friend class WLvlImpl_HotColdGCSwap_Erase;
    friend class GCImpl_DChoices;
    friend class GCImpl_DCh_HotCold;
    friend class GCImpl_DCh_TieErasures;
    friend class GCImpl_DCh_Erase;
    friend class GCImpl_DCh_Erase_TieValid;

	uint get_pages_valid(const Address& address) const;
	uint get_pages_invalid(const Address& address) const;
	uint get_pages_erased(const Address& address) const;
	ulong get_erases_remaining(const Address& address) const;
    ulong get_block_erase_count(const Address& address) const;
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
	virtual void modifyFTL(const ulong lpn,
						   const Address& newAddress); /// FIXME Use this instead of anonymous functions...
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

#ifndef NOT_USE_BLOCKMGR
class FtlImpl_DftlParent : public FtlParent
{
	public:
	FtlImpl_DftlParent(Controller& controller);
	~FtlImpl_DftlParent();
	virtual enum status read(Event& event) = 0;
	virtual enum status write(Event& event) = 0;
	virtual enum status trim(Event& event) = 0;

	protected:
	struct MPage
	{
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
				const FtlImpl_DftlParent::MPage&, double, &FtlImpl_DftlParent::mpage_last_visited_time_compare>>>>
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
	struct BPage
	{
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
#endif

class FtlImpl_SWF : public FtlParent // Simulates the working of a page-mapped FTL
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
	Address WF; // 1 WF
    Addresses map;
};

class FtlImpl_SWFWDiff : public FtlParent // Simulates the working of a page-mapped FTL with a limit on the maximum
                                          // difference in number of erasures
{
    class EraseWindow
    {
public:
        EraseWindow(const uint Wdiff, const Plane& pl);
        Address choose_Wmin_maxValid_BlockNr(const Address& victim, uint dSwap);
        void count_erase(const Address& block);
        Addresses get_addresses(const uint W) const;

        void _check_window(const Address& baseAddr);

        uint Wmin;
        uint Wmax;
        uint Wdiff;
        Plane& plane;

        std::vector<uint> eraseWindow;
        std::vector<uint> WIdx;
        std::vector<uint> blockNrToIdx;
    };

    public:
    FtlImpl_SWFWDiff(Controller& controller, /* HotColdID* hcID, */ const uint dSwap = HCSWAPWF_D);
    ~FtlImpl_SWFWDiff();
    void initialize(const ulong maxLPN);
    enum status read(Event& event);
    enum status write(Event& event);
    enum status trim(Event& event);

    private:
    void check_ftl_integrity(const ulong numLPN);
    Address WF; // 1 WF
    Addresses map;
    uint dSwap;
};

/// FIXME Change this to Address
class PlaneAddress
{
	public:
	uint package;
	uint die;
	uint plane;
	PlaneAddress(const Address& address); // : package(address.package), die(address.die), plane(address.plane) {}
	PlaneAddress(uint package, uint die, uint plane); // : package(package), die(die), plane(plane) {}
	uint to_linear_address() const;
};

/*template <>
class hash<PlaneAddress>
{
public:
		size_t operator()(ssd::PlaneAddress const& addr) const noexcept
		{
				return std::hash<uint>((addr.package * PACKAGE__SIZE + addr.die) * DIE_SIZE + addr.plane);
		}
};*/

class FtlImpl_DWF : public FtlParent
{
	public:
	FtlImpl_DWF(Controller& controller, HotColdID* hcID);
	// FtlImpl_DWF(Controller &controller, const std::vector<Event> &events);
	~FtlImpl_DWF();
	void initialize(const ulong maxLPN) override;
	enum status read(Event& event) override;
	enum status write(Event& event) override;
	enum status trim(Event& event) override;

	protected:
	void modifyFTL(const ulong lpn, const Address& address) override;
    /**
     * @brief gca_collect Calls the garbage collection algorithm.
     * @param event Event issuing the GCA invocation.
     * @param victimAddress Address of the victim block.
     * @param doNotPick Block addresses to ignore
     * @note victimAddress is used to determine which plane to search for a new victim block.
     */
    virtual void gca_collect(Event& event, Address& victimAddress, const Addresses& doNotPick);

	virtual void merge(Event& event, const Address& victim);

	void modify_ftl_page(const ulong lpn, const uint newPage);
	void recompute_hotvalidpages(Address block);
	void check_ftl_integrity(const ulong lpn);
    void check_ftl_integrity();
    void check_valid_pages();
	void check_hot_pages(Address blockAddr, Block* blockPtr, const uint hotPages);
    PlaneAddress lpnToPlane(ulong lpn);

	std::vector<Address> WFE; // External WF
	std::vector<Address> WFI; // Internal WF
    Addresses map;
	HotColdID* hcID;
	// std::vector<std::vector<std::vector<std::vector<uint>>>> hotValidPages;
};

class FtlImpl_DWFWDiff : public FtlImpl_DWF
{
    class EraseWindow
    {
public:
        EraseWindow(const uint Wdiff, const Plane& pl);
        Address choose_Wmin_maxValid_BlockNr(const Address& victim, uint dStar);
        void count_erase(const Address& block);
        // Addresses get_addresses(const uint W) const;

        // Addresses get_block_numbers(const uint W, const Address& baseAddr) const;
        Addresses get_block_addresses(const uint W, const Address& baseAddr) const;
        void _check_window(const Address& baseAddr, bool ignoreBaseAddr = true);

        uint Wmin;
        uint Wmax;
        uint Wdiff;
        Plane& plane;
        std::vector<uint> eraseWindow;
        std::vector<uint> WIdx;
        std::vector<uint> blockNrToIdx;
    };

    public:
    FtlImpl_DWFWDiff(Controller& controller, HotColdID* hcID, const uint dStar = HCSWAPWF_D,
                     const uint deltaW = MAX_ERASE_DIFF);
    ~FtlImpl_DWFWDiff();
    void initialize(const ulong maxLPN) override;

    private:
    void modifyFTL(const ulong lpn, const Address& address) override;
    virtual void gca_collect(Event& event, Address& victimAddress, const Addresses& doNotPick) override;
    virtual void merge(Event& event, const Address& victim) override;
    // Address choose_swap_block(const Address& victim);
    /// TODO Argument should be PlaneAddress, keep track of "forbidden" blocks per plane in FTL itself
    Address choose_move_block(const Address& victim);

    std::vector<EraseWindow> eraseWindows;
    uint dStar;
    uint deltaW;
};

class FtlImpl_DSwapWF : public FtlImpl_DWF
{
	public:
	FtlImpl_DSwapWF(Controller& controller, HotColdID* hcID, const double swapProbability = HCSWAPWF_PROBABILITY,
					const uint dSwap = HCSWAPWF_D);
    virtual ~FtlImpl_DSwapWF();
	void initialize(const ulong maxLPN) override;

	protected:
	/**
	 * @brief Choose a block to swap during GC in case the other WF has insufficient space.
	 * @param victim The currently selected victim block.
	 * @return Address of a block to swap contents with the other WF.
	 *
	 * @details The swap will not happen when the returned block and victim block have the same hotness.
	 */
	virtual Address choose_swap_block(const Address& victim);
	void check_ftl_hotness_integrity();
	void check_block_hotness();

	private:
	virtual void merge(Event& event, const Address& victim) override;
	enum status _erase_swap_and_copy(Event& event, Address& victimBlock, Block* victimPtr, Address& copyBlock,
									 Block* copyBlockPtr, Address& swapBlock,
									 std::function<void(const ulong, const Address&)> modifyFTL,
									 const bool replacingHWF, Stats& stats);
	uint dSwap;
	double pSwap;
	int numHotBlocks; // Marked as previously-WFE
};

class FtlImpl_HCWF : public FtlParent
{
	public:
	FtlImpl_HCWF(Controller& controller, HotColdID* hcID);
	virtual ~FtlImpl_HCWF();
	virtual void initialize(const ulong maxLPN);
	// void initialize(const std::set<ulong> &uniqueLPNs);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);

	protected:
	/**
	 * @brief modifyFTL Modifies the FTL mapping for a specific LPN
	 * @param lpn Logical page number that is invalidated
	 * @param address New physical address for lpn
	 */
	virtual void modifyFTL(const ulong lpn, const Address& address);
	/**
	 * @brief modifyFTLPage Modifies the FTL mapping for a specific LPN
	 * @param lpn Logical page number that is invalidated
	 * @param newPage New page number in physical address of lpn
	 * @warning The rest of the address (ssd, package, die, plane, block) in the mapping remains the same.
	 */
	virtual void modifyFTLPage(const ulong lpn, const uint newPage);

	///@TODO gca_collect should move to a call for block with minimal cost on a higher level (plane/die/package).
	/// GCA can then determine victim however it wants across packages.

	/**
	 * @brief gca_collect Calls the garbage collection algorithm.
	 * @param event Event issuing the GCA invocation.
	 * @param victimAddress Address of the victim block.
	 * @param replacingHWF
	 * @param doNotPick
	 * @note victimAddress is used to determine which plane to search for a new victim block.
	 */
    virtual void gca_collect(Event& event, Address& victimAddress, const bool replacingHWF, const Addresses& doNotPick);
	virtual void merge(Event& event, const Address& victim, const bool replacingHWF, Stats& stats);

	/**
	 * @brief lpnToPlane Computes the plane where data for an lpn should be written to.
	 * @param lpn Logical page number
	 * @return Address of plane where lpn-data belongs
	 */
	PlaneAddress lpnToPlane(ulong lpn);

	// Correctness verification
	void check_valid_pages(const ulong numLPN);
	void check_block_hotness();
    void check_ftl_hotness_integrity(const Event& evt);

	uint numHotBlocks;
	uint maxHotBlocks;
	std::vector<Address> HWF; // Hot WF
	std::vector<Address> CWF; // Cold WF
                              // std::map<ulong, Address> map;
    Addresses map;
	HotColdID* hcID;
	// std::vector<std::vector<std::vector<std::vector<bool>>>> blockIsHot;
};

/**
 * @brief The FtlImpl_HCWFFalse class
 * @deprecated
 */
class FtlImpl_HCWFFalse : public FtlImpl_HCWF
{
	public:
	FtlImpl_HCWFFalse(Controller& controller, HotColdID* hcID, double false_pos_prob = HCID_FALSE_POS_RATIO,
					  double false_neg_prob = HCID_FALSE_NEG_RATIO);
	~FtlImpl_HCWFFalse();
	enum status write(Event& event) override;

	private:
	double prob_fpos;
	double prob_fneg;
};

class FtlImpl_HCWFFixedFalse : public FtlImpl_HCWF
{
	public:
	FtlImpl_HCWFFixedFalse(Controller& controller, HotColdID* hcID, double false_pos_prob = HCID_FALSE_POS_RATIO,
						   double false_neg_prob = HCID_FALSE_NEG_RATIO);
	~FtlImpl_HCWFFixedFalse();
	void initialize(const ulong maxLPN) override;
	enum status write(Event& event) override;

	private:
	void merge(Event& event, const Address& victim, const bool replacingHWF, Stats& stats) override;

	double prob_fpos;
	double prob_fneg;
	ulong numHotLPN;
	ulong falseNegativeLimit;
	ulong falsePositiveLimit;
};

class FtlImpl_HCWFWDiff : public FtlImpl_HCWF
{
    public:
    FtlImpl_HCWFWDiff(Controller& controller, HotColdID* hcID, const uint dSwap = HCSWAPWF_D,
                      const uint Wdiff = MAX_ERASE_DIFF);
    virtual ~FtlImpl_HCWFWDiff();
    virtual void initialize(const ulong numLPN) override;

    private:
    virtual void gca_collect(Event& event, Address& victim, const bool replacingHWF,
                             const std::vector<Address>& doNotPick) override;
    virtual void merge(Event& event, const Address& victim, const bool replacingHWF, Stats& stats) override;
    enum status _erase_swap_and_copy(Event& event, Address& victimBlock, Block* victimPtr, Address& copyBlock,
                                     Block* copyBlockPtr, Address& swapBlock,
                                     std::function<void(const ulong, const Address&)> modifyFTL,
                                     const bool replacingHWF, Stats& stats);
    Addresses& get_erase_window(const ulong& pLinAddr, const uint W);
    /**
     * @brief Choose a block to swap during GC in case the other WF has insufficient space.
     * @param victim The currently selected victim block.
     * @return Address of a block to swap contents with the other WF.
     *
     * @details The swap will not happen when the returned block and victim block have the same hotness.
     */
    virtual Address choose_swap_block(const Address& victim, bool replacingHWF);

    uint dSwap;
    // The following all have 1 for each plane
    std::vector<std::vector<Addresses>> eraseWindows; // consisting of Wmax-Wmin vectors of Addresses
    std::vector<std::pair<uint, uint>> Wminmax;
    std::vector<uint> WminIdx;
};

class FtlImpl_HCSwapWF : public FtlImpl_HCWF
{
	public:
	FtlImpl_HCSwapWF(Controller& controller, HotColdID* hcID, const double swapProbability = HCSWAPWF_PROBABILITY,
					 const uint dSwap = HCSWAPWF_D);
	virtual ~FtlImpl_HCSwapWF();

	protected:
	/**
	 * @brief Choose a block to swap during GC in case the other WF has insufficient space.
	 * @param victim The currently selected victim block.
	 * @return Address of a block to swap contents with the other WF.
	 *
	 * @details The swap will not happen when the returned block and victim block have the same hotness.
	 */
    virtual Address choose_swap_block(const Address& victim, bool replacingHWF);

	uint dSwap;

	private:
	virtual void merge(Event& event, const Address& victim, const bool replacingHWF, Stats& stats) override;
	enum status _erase_swap_and_copy(Event& event, Address& victimBlock, Block* victimPtr, Address& copyBlock,
									 Block* copyBlockPtr, Address& swapBlock,
									 std::function<void(const ulong, const Address&)> modifyFTL,
									 const bool replacingHWF, Stats& stats);
	double pSwap;
};

class FtlImpl_HCSwapWF_EraseTie : public FtlImpl_HCSwapWF
{
    public:
    FtlImpl_HCSwapWF_EraseTie(Controller& controller, HotColdID* hcID,
                              const double swapProbability = HCSWAPWF_PROBABILITY, const uint dSwap = HCSWAPWF_D);
    virtual ~FtlImpl_HCSwapWF_EraseTie();

    protected:
    /**
     * @brief Choose a block to swap during GC in case the other WF has insufficient space.
     * @param victim The currently selected victim block.
     * @return Address of a block to swap contents with the other WF.
     *
     * @details The swap will not happen when the returned block and victim block have the same hotness.
     */
    virtual Address choose_swap_block(const Address& victim, bool replacingHWF);

    private:
};

class FtlImpl_HCSwapWF_HCEraseTie : public FtlImpl_HCSwapWF
{
    public:
    FtlImpl_HCSwapWF_HCEraseTie(Controller& controller, HotColdID* hcID,
                                const double swapProbability = HCSWAPWF_PROBABILITY, const uint dSwap = HCSWAPWF_D);
    virtual ~FtlImpl_HCSwapWF_HCEraseTie();

    protected:
    /**
     * @brief Choose a block to swap during GC in case the other WF has insufficient space.
     * @param victim The currently selected victim block.
     * @return Address of a block to swap contents with the other WF.
     *
     * @details The swap will not happen when the returned block and victim block have the same hotness.
     */
    virtual Address choose_swap_block(const Address& victim, bool replacingHWF);

    private:
};

class FtlImpl_HCWFPlusSwap : public FtlImpl_HCSwapWF
{
	public:
	FtlImpl_HCWFPlusSwap(Controller& controller, HotColdID* hcID, const double swapProbability = HCSWAPWF_PROBABILITY,
						 const uint dSwap = HCSWAPWF_D);
	virtual ~FtlImpl_HCWFPlusSwap();

	protected:
    virtual Address choose_swap_block(const Address& victim, bool replacingHWF) override;
};

class FtlImpl_HCSwapWF_Erase : public FtlImpl_HCSwapWF
{
    public:
    FtlImpl_HCSwapWF_Erase(Controller& controller, HotColdID* hcID, const double swapProbability = HCSWAPWF_PROBABILITY,
                           const uint dSwap = HCSWAPWF_D);
    virtual ~FtlImpl_HCSwapWF_Erase();

    protected:
    /**
     * @brief Choose a block to swap during GC in case the other WF has insufficient space.
     * @param victim The currently selected victim block.
     * @return Address of a block to swap contents with the other WF.
     *
     * @details The swap will not happen when the returned block and victim block have the same hotness.
     */
    virtual Address choose_swap_block(const Address& victim, bool replacingHWF);

    private:
};

class FtlImpl_HCSwapWF_Erase_ValidTie : public FtlImpl_HCSwapWF
{
    public:
    FtlImpl_HCSwapWF_Erase_ValidTie(Controller& controller, HotColdID* hcID,
                                    const double swapProbability = HCSWAPWF_PROBABILITY, const uint dSwap = HCSWAPWF_D);
    virtual ~FtlImpl_HCSwapWF_Erase_ValidTie();

    protected:
    /**
     * @brief Choose a block to swap during GC in case the other WF has insufficient space.
     * @param victim The currently selected victim block.
     * @return Address of a block to swap contents with the other WF.
     *
     * @details The swap will not happen when the returned block and victim block have the same hotness.
     */
    virtual Address choose_swap_block(const Address& victim, bool replacingHWF);

    private:
};

class FtlImpl_HCWF_DHC : public FtlImpl_HCWF
{
	public:
	FtlImpl_HCWF_DHC(Controller& controller, HotColdID* hcID, const uint d_HWF = DCHOICES_DH,
					 const uint d_CWF = DCHOICES_DC);
	~FtlImpl_HCWF_DHC();

	protected:
	virtual void gca_collect(Event& event, Address& victimAddress, const bool replacingHWF,
                             const Addresses& doNotPick) override;

	private:
	uint dH;
	uint dC;
};

class FtlImpl_COLD : public FtlImpl_HCWF
{
	public:
	FtlImpl_COLD(Controller& controller, HotColdID* hcID, const uint d = DCHOICES_D);
	~FtlImpl_COLD();

	protected:
	virtual void gca_collect(Event& event, Address& victimAddress, const bool replacingHWF,
                             const Addresses& doNotPick) override;

	private:
	uint d;
};

class FtlImpl_COLD_DHC : public FtlImpl_HCWF
{
	public:
	FtlImpl_COLD_DHC(Controller& controller, HotColdID* hcID, const uint d_HWF = DCHOICES_DH,
					 const uint d_CWF = DCHOICES_DC);
	~FtlImpl_COLD_DHC();

	protected:
	virtual void gca_collect(Event& event, Address& victimAddress, const bool replacingHWF,
                             const Addresses& doNotPick) override;

	private:
	uint dH;
	uint dC;
};

class FtlImpl_STAT : public FtlParent
{
	public:
	FtlImpl_STAT(Controller& controller, HotColdID* hcID, const double p = HOT_SPARE_FACTOR_FRACTION);
	~FtlImpl_STAT();
	virtual void initialize(const ulong maxLPN);
	// void initialize(const std::set<ulong> &uniqueLPNs);
	enum status read(Event& event);
	enum status write(Event& event);
	enum status trim(Event& event);

	protected:
	void modifyFTL(const ulong lpn, const Address& address);
	void modifyFTLPage(const ulong lpn, const uint newPage);

	private:
	PlaneAddress lpnToPlane(ulong lpn);
	// Correctness verification
	void check_valid_pages(const ulong numLPN);
	void check_block_hotness();
	void check_ftl_hotness_integrity();

	uint numHotBlocks;
	uint maxHotBlocks;
	uint maxHotBlocksPerPlane;
	double p;
	std::vector<Address> HWF; // Hot WF
	std::vector<Address> CWF; // Cold WF
    Addresses map;
	HotColdID* hcID;
};

/* This is a basic implementation that only provides delay updates to events
 * based on a delay value multiplied by the size (number of pages) needed to
 * be written. */
class Ram
{
	public:
	Ram(double read_delay = RAM_READ_DELAY, double write_delay = RAM_WRITE_DELAY);
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
    ulong get_block_erase_count(const Address& address) const;

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
	double event_arrive(enum event_type type, ulong logical_address, uint size, double start_time);
	double event_arrive(enum event_type type, ulong logical_address, uint size, double start_time, void* buffer);
	friend class Controller;
	void print_statistics();
	void reset_statistics();
	void write_statistics(FILE* stream);
	void write_statistics_csv(const std::string fileName, const uint runID);
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
    ulong get_block_erase_count(const Address& address) const;

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
	void get_free_page(Address& address) const; /// TODO What is this in relation to get_next_page?
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
	double event_arrive(enum event_type type, ulong logical_address, uint size, double start_time);
	double event_arrive(enum event_type type, ulong logical_address, uint size, double start_time, void* buffer);
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

/**
 * Checks whether a block is in a range of blocks within a plane.
 * @param[in] block Address of the block to check
 * @param[in] beginBlockAddress Begin of block address range
 * @param[in] endBlockAddress End of block address range
 * @return True if block address is greater than or equal to beginBlockAddress and less than or equal to
 * endBlockAddress.
 */
bool block_is_in_plane_range(const Address& block, const Address& beginBlockAddress, const Address& endBlockAddress);

/**
 * Checks whether a block is in a vector of blocks
 * @param[in] block Address of the block
 * @param[in] vector Vector of block addresses
 * @return True if block is in vector
 */
bool block_is_in_vector(const Address& block, const Addresses& vector);
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
void random_block(Address& victimAddress, const Addresses& doNotPick);

/**
 * Selects a block in a uniform random fashion.
 * @param[out] victimAddress Address that is selected
 * @param[in] ignorePredicate Block is not selected as one of the d choices if
 * this evaluates to true.
 */
void random_block(Address& victimAddress, const std::function<bool(const Address&)>& ignorePredicate);

/// TODO Update documentation
/**
 * Selects a block in a uniform random fashion.
 * @param[out] victimAddress Address that is selected
 */
void random_block_same_plane(Address& victimAddress);

/**
 * Selects a block in a uniform random fashion.
 * @param[out] victimAddress Address that is selected
 * @param[in] doNotPick Addresses of blocks that are not to be selected.
 */
void random_block_same_plane(Address& victimAddress, const Addresses& doNotPick);

/**
 * Selects a block in a uniform random fashion.
 * @param[out] victimAddress Address that is selected
 * @param[in] ignorePredicate Block is not selected as one of the d choices if
 * this evaluates to true.
 */
void random_block_same_plane(Address& victimAddress, const std::function<bool(const Address&)>& ignorePredicate);

///**
// * Selects the block with minimal cost out of @p d randomly chosen blocks.
// * @tparam T Return type of the cost function
// * @param[in] d Number of blocks (uniform) randomly selected.
// * @param[inout] victimAddress Address that is selected.
// * @details At return, victimAddress contains an address located in the same plane as its input value.
// * @param[in] costToMinimise Cost function to be minimised
// * @param[in] ignorePredicate Block is not selected as one of the d choices if
// * this evaluates to true.
// * @details Requires the < operator to be defined for type T
// */
// template <typename T>
// void d_choices_block_same_plane(const unsigned int d, Address& victimAddress,
//                                const std::function<T(const Address&)> costToMinimise,
//                                const std::function<bool(const Address&)>& ignored)
//{
//    Address tmpAddr{victimAddress};

//    T minCost = std::numeric_limits<T>::max();
//    auto minCostBlockNr = tmpAddr.block;
//    uint tries = 0;
//    while (tries < d) {
//        const uint blockNr = RandNrGen::get(PLANE_SIZE);
//        tmpAddr.block = blockNr;
//        const T blockCost = costToMinimise(tmpAddr); // plane->data[blockNr].pages_valid;
//        tmpAddr.block = blockNr;
//        if (not ignored(tmpAddr)) {
//            tries++;
//            if (blockCost < minCost) {
//                minCost = blockCost;
//                minCostBlockNr = blockNr;
//            }
//        }
//    }
//    victimAddress.block = minCostBlockNr;
//}

///**
// * Selects the block with minimal cost out of @p d randomly chosen blocks, with a tie breaker in case of equal cost.
// * @tparam T Return type of the cost function
// * @tparam T2 Return type of the tie breaking cost function
// * @param[in] d Number of blocks (uniform) randomly selected.
// * @param[inout] victimAddress Address that is selected.
// * @details At return, victimAddress contains an address located in the same plane as its input value.
// * @param[in] costToMinimise Cost function to be minimised
// * @param[in] tieBreakerToMinimise Tie breaking cost function, to be minimised
// * @param[in] ignorePredicate Block is not selected as one of the d choices if
// * this evaluates to true.
// * @details Requires the < operator to be defined for types T and T2
// */
// template <typename T, typename T2>
// void d_choices_block_same_plane_tiebreak(const unsigned int d, Address& victimAddress,
//                                         const std::function<T(const Address&)> costToMinimise,
//                                         const std::function<T2(const Address&)> tieBreakerToMinimise,
//                                         const std::function<bool(const Address&)>& ignored)
//{
//    Address tmpAddr{victimAddress};

//    uint minCost = std::numeric_limits<uint>::max();
//    std::vector<uint> minCostBlockNumbers = {};
//    T2 minTBCost = std::numeric_limits<T2>::max();

//    uint tries = 0;
//    while (tries < d) {
//        const uint blockNr = RandNrGen::get(PLANE_SIZE);
//        tmpAddr.block = blockNr;
//        const T blCost = costToMinimise(tmpAddr);		   // plane->data[blockNr].pages_valid;
//        const T2 blTBCost = tieBreakerToMinimise(tmpAddr); // plane->data[blockNr].get_erase_count();

//        tmpAddr.block = blockNr;
//        if (not ignored(tmpAddr)) {
//            tries++;
//            const bool lessCost = blCost < minCost;
//            const bool sameCost = blCost == minCost;
//            const bool sameTie = blTBCost == minTBCost;
//            if (sameCost and sameTie) {
//                // Select the block with the least amount of erases
//                minCostBlockNumbers.push_back(blockNr);
//            } else if (lessCost or (sameCost and blTBCost < minTBCost)) {
//                minCostBlockNumbers.clear();
//                minCostBlockNumbers.push_back(blockNr);
//                minCost = blCost;
//                minTBCost = blTBCost;
//            }
//        }
//    }
//    victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
//}

///**
// * Selects the block with minimal cost out of all blocks in a plane.
// * @tparam T Return type of the cost function
// * @param[inout] victimAddress Address that is selected.
// * @details At return, victimAddress contains an address located in the same plane as its input value.
// * @param[in] costToMinimise Cost function to be minimised
// * @param[in] ignorePredicate Block is not selected as one of the d choices if
// * this evaluates to true.
// * @details Requires the < operator to be defined for type T
// */
// template <typename T>
// void greedy_block_same_plane(Address& victimAddress, const std::function<T(const Address&)> costToMinimise,
//                             const std::function<bool(const Address&)>& ignored)
//{
//    /** Corrected: pick the minimum block
//        if a block is encountered with the same number of valid pages, decide whether to change or not
//        This should lead to equal probability to get chosen for all blocks with the same amount of valid pages... *//*
//    ssd::Plane* plane = ctrl.get_plane_pointer(victimAddress);
//    Address address(victimAddress);

//    uint minCost = std::numeric_limits<uint>::max();
//    auto minCostBlockNr = address.block;
//    auto minCounter = 1;
//    for (auto blockNr = 0; blockNr < PLANE_SIZE; blockNr++) {
//        const uint blockCost = plane->data[blockNr].pages_valid;
//        address.block = blockNr;
//        if (not ignored(address)) {

//            if (blockCost < minCost) {
//                minCounter = 1;
//                minCost = blockCost;
//                minCostBlockNr = blockNr;
//            } else if (blockCost == minCost) {
//                minCounter++;
//                if (RandNrGen::get() < 1.0 / minCounter) {
//                    minCostBlockNr = blockNr;
//                }
//            }
//        }
//    }
//    victimAddress.block = minCostBlockNr;*/
//    /**Naive way*/
//    Address tmpAddr{victimAddress};

//    T minCost = std::numeric_limits<T>::max();
//    std::vector<uint> minCostBlockNumbers = {};
//    for (uint blockNr = 0; blockNr < PLANE_SIZE; blockNr++) {
//        tmpAddr.block = blockNr;
//        const T blCost = costToMinimise(tmpAddr);
//        if (not ignored(tmpAddr)) {
//            if (blCost < minCost) {
//                minCostBlockNumbers.clear();
//                minCostBlockNumbers.push_back(blockNr);
//                minCost = blCost;
//            } else if (blCost == minCost) {
//                minCostBlockNumbers.push_back(blockNr);
//            }
//        }
//    }
//    victimAddress.block = minCostBlockNumbers[RandNrGen::get(minCostBlockNumbers.size())];
//}

/**
 * Selects the block with minimal valid pages out of d randomly chosen blocks on same plane as the input address.
 * @param[in] ctrl Controller of SSD
 * @param[in] d Number of blocks (uniform) randomly selected.
 * @param[inout] victimAddress Address that is selected.
 *      Every address component including plane-level and above remains the same.
 * @param[in] ignorePredicate Block is not selected as one of the d choices if
 * this evaluates to true.
 * @returns Address of chosen block
 */
void d_choices_block_same_plane_min_valid_pages(Controller& ctrl, const unsigned int d, Address& victimAddress,
                                                const std::function<bool(const Address&)>& ignored);

/**
 * Selects the block with minimal valid pages out of d randomly chosen blocks on same plane as the input address.
 * @param[in] ctrl Controller of SSD
 * @param[in] d Number of blocks (uniform) randomly selected.
 * @param[inout] victimAddress Address that is selected.
 *      Every address component including plane-level and above remains the same.
 * @param[in] ignorePredicate Block is not selected as one of the d choices if
 * this evaluates to true.
 * @returns Address of chosen block
 */
void d_choices_block_same_plane_min_valid_pages_tie_min_erase(Controller& ctrl, const unsigned int d,
                                                              Address& victimAddress,
                                                              const std::function<bool(const Address&)>& ignored);
void d_choices_block_same_plane_min_valid_pages_tie_max_erase(Controller& ctrl, const uint d, Address& victimAddress,
                                                              const std::function<bool(const Address&)>& ignored);

/**
 * Selects the block with minimal cost out of d randomly chosen blocks on same plane as the input address.
 * @tparam Return type of the cost function
 * @param[in] d Number of blocks (uniform) randomly selected.
 * @param[inout] victimAddress Address that is selected.
 *      Every address component including plane-level and above remains the same.
 * @param[in] costFunction Cost function to be minimised
 * @param[in] ignorePredicate Block is not selected as one of the d choices if
 * this evaluates to true.
 * @details Requires the < operator to be defined for type T
 */
void d_choices_block_same_plane(const unsigned int d, Address& victimAddress,
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
void greedy_block(Address& victimAddress, const std::function<uint(const Address&)>& costFunction,
                  const std::function<bool(const Address&)>& ignorePredicate);

void greedy_block(Address& victimAddress, const std::function<double(const Address&)>& costFunction,
                  const std::function<bool(const Address&)>& ignorePredicate);
void greedy_block_same_plane(Address& victimAddress, const std::function<uint(const Address&)>& costFunction,
                             const std::function<bool(const Address&)>& ignorePredicate);

void greedy_block_same_plane(Address& victimAddress, const std::function<double(const Address&)>& costFunction,
                             const std::function<bool(const Address&)>& ignorePredicate);

void greedy_block_same_plane_min_valid_pages(Controller& ctrl, Address& victimAddress,
                                             const std::function<bool(const Address&)>& ignored);

void greedy_block_same_plane_min_valid_pages_tie_min_erase(Controller& ctrl, Address& victimAddress,
                                                           const std::function<bool(const Address&)>& ignored);

void greedy_block_same_plane_min_valid_pages_tie_max_erase(Controller& ctrl, Address& victimAddress,
                                                           const std::function<bool(const Address&)>& ignored);

void d_choices_block_same_plane_min_erase(Controller& ctrl, const unsigned int d, Address& victimAddress,
                                          const std::function<bool(const Address&)>& ignored);
void greedy_block_same_plane_min_erase(Controller& ctrl, Address& victimAddress,
                                       const std::function<bool(const Address&)>& ignored);
void d_choices_block_same_plane_min_erase_tie_valid(Controller& ctrl, const unsigned int d, Address& victimAddress,
                                                    const std::function<bool(const Address&)>& ignored);
void greedy_block_same_plane_min_erase_tie_valid(Controller& ctrl, Address& victimAddress,
                                                 const std::function<bool(const Address&)>& ignored);

} /* end namespace ssd */

#endif
