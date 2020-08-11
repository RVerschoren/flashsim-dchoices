#ifndef SSD_RUN_H
#define SSD_RUN_H

#include "ssd.h"
#include <string>

namespace ssd {

struct SSD_Run_Params
{
	// Shared parameters
	std::string out_filename;
	uint run_start;
	uint nr_runs;
	ulong max_PE;

	// Synthetic parameters
	ulong nr_LPN;
	ulong max_hot_LPN;
	ulong max_cold_LPN;

	// Trace parameters
	std::string trace_file;
	std::string traceID;
	ssd::EVENT_READER_MODE read_mode;
	ulong nr_events;
	std::vector<IOEvent> events;
};

/**
 * @brief setup_synth Reads command line arguments and sets the system parameters.
 * @param argc Number of arguments
 * @param argv Command line arguments
 * @param verbose Prints information about read parameters
 * @return SSD simulation parameters
 */
SSD_Run_Params setup_synth(int argc, char* argv[], ssd::wl_scheme wlvl = WL_NONE);

/**
 * @brief setup_synth Reads command line arguments and sets the system parameters.
 * @param args Command line arguments
 * @param verbose Prints information about read parameters
 * @return SSD simulation parameters
 */
SSD_Run_Params setup_synth(const std::vector<std::string>& args, ssd::wl_scheme wlvl = WL_NONE);

void run_synth(const SSD_Run_Params& params);

void run_synth(const SSD_Run_Params& params, double& WA, std::vector<double>& endurance, std::vector<double>& fairness,
			   std::vector<double>& hotbl, std::vector<double>& coldbl);

SSD_Run_Params setup_trace(int argc, char* argv[], ssd::wl_scheme wlvl = WL_NONE);

SSD_Run_Params setup_trace(const std::vector<std::string>& args, ssd::wl_scheme wlvl = WL_NONE);

void run_trace(const SSD_Run_Params& params);

} // namespace ssd
#endif // SSD_RUN_H
