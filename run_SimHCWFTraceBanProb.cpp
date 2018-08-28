/* Trace simulations with DWF
 * Robin Verschoren 2017
 */

#include "ssd.h"
#include <iomanip>
#include <iostream>

using namespace ssd;

int
main(int argc, char* argv[])
{
    load_config();
    /// Override with command line settings
    BLOCK_SIZE = std::stoi(argv[1]);
    DCHOICES_D = std::stoi(argv[2]);
    SPARE_FACTOR = std::stof(argv[3]);
    HOT_FRACTION = std::stof(argv[4]);        // f
    const uint startrun = std::stoi(argv[5]); // startrun
    const uint nruns = std::stoi(argv[6]);    // Number of runs
    BLOCK_ERASES = 10000000;
    const ulong maxPE = BLOCK_ERASES - std::stoi(argv[7]); // Maximum number of PE cycles, counting back
    const ulong numRequests = std::stol(argv[8]);
    const std::string traceFile(argv[9]);
    const std::string traceID = traceFile.substr(0, 4);

    EVENT_READER_MODE mode;
    switch (std::stoi(argv[10])) {
        case 0:
            mode = EVTRDR_SIMPLE;
            break;
        case 1:
            mode = EVTRDR_BIOTRACER;
            break;
        default:
            mode = EVTRDR_SIMPLE;
            break;
    }
    WLVL_BAN_P = std::stod(argv[11]); // Ban's algorithm p parameter
    std::cout << WLVL_BAN_P << std::endl;
    WEAR_LEVELER = WL_BAN_PROB;

    uint nrFrames = 1;

    // HCWF
    FTL_IMPLEMENTATION = IMPL_HCWF;
    // DChoices
    GC_ALGORITHM = DCHOICES;


    EventReader evtRdr(traceFile, numRequests, mode, create_oracle_filename(traceID, HOT_FRACTION, nrFrames));
    std::vector<IOEvent> events = evtRdr.read_IO_events_from_trace(traceFile);
    const ulong maxLPN = evtRdr.find_max_lpn(events);
    const ulong numLPN = maxLPN + 1;

    PLANE_SIZE = std::ceil((double)numLPN) / (double)(BLOCK_SIZE * (1.0 - SPARE_FACTOR));

    std::stringstream sstr;
    sstr << std::fixed; // Print trailing zeroes
    sstr << "dwftraceban";
    sstr << "-b" << BLOCK_SIZE;
    sstr << "-d" << DCHOICES_D;
    sstr << "-sf" << std::setprecision(2) << SPARE_FACTOR;
    sstr << "-f" << std::setprecision(3) << HOT_FRACTION;
    sstr << "-p" << std::setprecision(3) << WLVL_BAN_P;
    sstr << "-" << traceID;
    std::string fileName = sstr.str();

    for (uint run = startrun; run < (startrun + nruns); run++) {
        RandNrGen::reset();
        HotColdID* hcID = new Static_HCID(maxLPN);

        Ssd ssd(SSD_SIZE, hcID);
        // ssd.initialize(uniqueLPNs);
        ssd.initialize(maxLPN);

        const Controller& ctrl = ssd.get_controller();
        uint it = 0;
        double startTime = 0.0;
        while (ctrl.stats.get_currentPE() > maxPE) {
            const IOEvent& evt = events[it];
            ssd.event_arrive(evt.type, evt.lpn, evt.size, startTime); // Timings don't really matter for PE
                                                                      // fairness/SSD endurance
            it = (it + 1) % numRequests;
            startTime += 5 * ((evt.type == READ) ? PAGE_READ_DELAY : PAGE_WRITE_DELAY);
        }
        ssd.write_statistics_csv(fileName, run);
        delete hcID;
    }
    return 0;
}
