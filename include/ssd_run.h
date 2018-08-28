#ifndef SSD_RUN_H
#define SSD_RUN_H

#include "ssd.h"
#include <string>

namespace ssd
{

struct SSD_Run_Params{
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
 * @param arg Command line argument
 * @return String description of FTL, for use in filenames
 */
SSD_Run_Params
setup_synth(int argc, char* argv[], std::string wlvlstr = "", ssd::wl_scheme wlvl = WL_NONE);

void
run_synth(const SSD_Run_Params& params);

SSD_Run_Params
setup_trace(int argc, char* argv[], std::string wlvlstr = "", ssd::wl_scheme wlvl = WL_NONE);

void
run_trace(const SSD_Run_Params& params);

}
#endif // SSD_RUN_H
