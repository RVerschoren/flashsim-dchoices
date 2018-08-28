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
    uint argit = 1;
    const std::string ftl(argv[argit++]);
    BLOCK_SIZE = std::stoi(argv[argit++]);
    DCHOICES_D = std::stoi(argv[argit++]);
    SPARE_FACTOR = std::stof(argv[argit++]);
    HOT_FRACTION = std::stof(argv[argit++]);        // f
    const uint startrun = std::stoi(argv[argit++]); // startrun
    const uint nruns = std::stoi(argv[argit++]);    // Number of runs
    BLOCK_ERASES = 10000000;
    const ulong maxPE = BLOCK_ERASES - std::stoi(argv[argit++]); // Maximum number of PE cycles, counting back
    const ulong numRequests = std::stol(argv[argit++]);
    const std::string traceFile(argv[argit++]);
    const std::string traceID = traceFile.substr(0, 4);

    EVENT_READER_MODE mode;
    switch (std::stoi(argv[argit++])) {
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
    WLVL_BAN_P = std::stod(argv[argit++]); // Ban's algorithm p parameter
    std::cout << WLVL_BAN_P << std::endl;
    WEAR_LEVELER = WL_BAN_PROB;

    if(true){
        // HCWF
        FTL_IMPLEMENTATION = IMPL_HCWF;
    }else if(){
    }



    uint nrFrames = 1;

    // DChoices
    GC_ALGORITHM = DCHOICES;

    EventReader evtRdr(traceFile, numRequests, mode, create_oracle_filename(traceID, HOT_FRACTION, nrFrames));
    std::vector<IOEvent> events = evtRdr.read_IO_events_from_trace(traceFile);
    const ulong maxLPN = evtRdr.find_max_lpn(events);
    const ulong numLPN = maxLPN + 1;

    PLANE_SIZE = std::ceil(static_cast<double>(numLPN) / static_cast<double>(BLOCK_SIZE * (1.0 - SPARE_FACTOR)));

    std::stringstream sstr;
    sstr << std::fixed; // Print trailing zeroes
    sstr << "hcwftraceban";
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
        ssd.initialize(numLPN);

        const Controller& ctrl = ssd.get_controller();
        uint it = 0;
        double startTime = 0.0;
        while (ctrl.stats.get_currentPE() > maxPE) {
            if (BLOCK_ERASES - ctrl.stats.get_currentPE() % 100 == 0) {
                std::cout << "CURRENT PE" << BLOCK_ERASES - ctrl.stats.get_currentPE() << std::endl;
            }
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
