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
    BLOCK_SIZE = std::atoi(argv[1]);
    DCHOICES_D = std::atoi(argv[2]);
    SPARE_FACTOR = std::atof(argv[3]);
    HOT_FRACTION = std::atof(argv[4]);        // f
    const uint startrun = std::atoi(argv[5]); // startrun
    const uint nruns = std::atoi(argv[6]);    // Number of runs
    BLOCK_ERASES = 10000000;
    const ulong maxPE =
      BLOCK_ERASES -
      std::atoi(argv[7]); // Maximum number of PE cycles, counting back
    const ulong numRequests = std::atol(argv[8]);
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

    uint nrFrames = 1;

    // DWF
    FTL_IMPLEMENTATION = 6;
    // DChoices
    GC_ALGORITHM = 3;

    // std::vector<Event> events = read_event_from_trace(traceFile,
    // readEvtFunc);
    // assert(events.size() == numRequests);
    // const ulong numUniqueLPNs = count_unique_lpns(events);

    EventReader evtRdr(traceFile, numRequests, mode,
                       create_oracle_filename(traceID, HOT_FRACTION, nrFrames));
    std::vector<IOEvent> events = evtRdr.read_IO_events_from_trace(traceFile);
    std::set<ulong> uniqueLPNs = evtRdr.read_accessed_lpns(events);
    // std::set<ulong> hotLPNs = evtRdr.read_hot_lpns();
    const ulong maxLPN =
      *std::max_element(uniqueLPNs.begin(), uniqueLPNs.end());
    assert(maxLPN == (uniqueLPNs.size() - 1));

    PLANE_SIZE = std::ceil((double)uniqueLPNs.size() /
                           (double)(BLOCK_SIZE * (1.0 - SPARE_FACTOR)));

    // Example format filename :
    // 'dwf-b',I2,'-d',I3,'-rho',F4.2,'-r',F5.3,'-f',F5.3,'-WA.',I2,'.csv';
    std::stringstream sstr;
    sstr << std::fixed; // Print trailing zeroes
    sstr << "dwftrace";
    sstr << "-b" << std::setw(2) << BLOCK_SIZE;
    sstr << "-d" << std::setw(3) << DCHOICES_D;
    sstr << "-sf" << std::setw(4) << std::setprecision(2) << SPARE_FACTOR;
    sstr << "-f" << std::setw(5) << std::setprecision(3) << HOT_FRACTION;
    sstr << "-" << traceID;
    std::string fileName = sstr.str();

    for (uint run = startrun; run < (startrun + nruns); run++) {
        RandNrGen::reset();
        // HotColdID *hcID = new Oracle_HCID(hotLPNs);
        HotColdID* hcID = new Static_HCID(maxLPN);

        Ssd ssd(SSD_SIZE, hcID);
        // ssd.initialize(uniqueLPNs);
        ssd.initialize(maxLPN);

        const Controller& ctrl = ssd.get_controller();
        uint it = 0;
        double startTime = 0.0;
        while (ctrl.stats.get_currentPE() > maxPE) {
            const IOEvent& evt = events[it];
            ssd.event_arrive(evt.type, evt.lpn, evt.size,
                             startTime); // Timings don't really matter for PE
                                         // fairness/SSD endurance
            it = (it + 1) % numRequests;
            startTime +=
              5 * ((evt.type == READ) ? PAGE_READ_DELAY : PAGE_WRITE_DELAY);
        }
        ssd.write_statistics_csv(fileName, run);
        delete hcID;
    }
    return 0;
}
