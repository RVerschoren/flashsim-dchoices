/* Trace simulations with DWF
 * Robin Verschoren 2017
 */

#include "ssd.h"
#include "ssd_run.h"

using namespace ssd;

int
main(int argc, char* argv[])
{
    load_config();

    /// Override with command line settings
    const SSD_Run_Params params = setup_trace(argc, argv, "maxval", WL_MAXVALID);
    ssd::run_trace(params);
    return 0;
}

//uint STARTRUN;
//uint NRUNS;
//ulong MAXPE;
//ulong NUMREQUESTS;
//std::string TRACEFILE;
//EVENT_READER_MODE READER_MODE;
//
///**
// * @brief set_FTL Sets
// * @param arg Command line argument
// * @return String description of FTL, for use in filenames
// */
//std::string
//setup(int /*argc*/, char* argv[])
//{
//    uint argit = 1;
//    const std::string ftl(argv[argit++]);
//    BLOCK_SIZE = static_cast<uint>(std::stoi(argv[argit++]));
//    DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
//    SPARE_FACTOR = std::stod(argv[argit++]);
//
//    std::string ftlstr;
//    if (boost::iequals(ftl, "DWF")) {
//        ftlstr = "dwf";
//        FTL_IMPLEMENTATION = IMPL_DWF;
//        HOT_FRACTION = std::stod(argv[argit++]); // f
//    } else if (boost::iequals(ftl, "HCWF")) {
//        ftlstr = "hcwf";
//        FTL_IMPLEMENTATION = IMPL_HCWF;
//        HOT_FRACTION = std::stod(argv[argit++]); // f
//    } else if (boost::iequals(ftl, "COLD")) {
//        ftlstr = "cold";
//        FTL_IMPLEMENTATION = IMPL_COLD;
//        HOT_FRACTION = std::stod(argv[argit++]); // f
//    } else {
//        // SWF
//        ftlstr = "swf";
//        FTL_IMPLEMENTATION = IMPL_SWF;
//        HOT_FRACTION = 0.0;
//    }
//
//    STARTRUN = static_cast<uint>(std::stoi(argv[argit++])); // startrun
//    NRUNS = static_cast<uint>(std::stoi(argv[argit++]));    // Number of runs
//    BLOCK_ERASES = 10000000;
//    MAXPE = BLOCK_ERASES -
//            static_cast<uint>(std::stoi(
//              argv[argit++])); // Maximum number of PE cycles, counting back
//    NUMREQUESTS = std::stoul(argv[argit++]);
//    TRACEFILE = argv[argit++];
//    const std::string traceID = TRACEFILE.substr(0, 4);
//
//    switch (std::stoi(argv[argit++])) {
//        case 0:
//            READER_MODE = EVTRDR_SIMPLE;
//            break;
//        case 1:
//            READER_MODE = EVTRDR_BIOTRACER;
//            break;
//        default:
//            READER_MODE = EVTRDR_SIMPLE;
//            break;
//    }
//    WLVL_ACTIVATION_PROBABILITY =
//      std::stod(argv[argit++]); // Ban's algorithm p parameter
//    std::cout << WLVL_ACTIVATION_PROBABILITY << std::endl;
//    WEAR_LEVELER = WL_MAXVALID;
//
//    // DChoices
//    GC_ALGORITHM = DCHOICES;
//
//    std::stringstream sstr;
//    sstr << std::fixed; // Print trailing zeroes
//    sstr << ftlstr << "tracemaxval";
//    sstr << "-b" << BLOCK_SIZE;
//    sstr << "-d" << DCHOICES_D;
//    sstr << "-sf" << std::setprecision(2) << SPARE_FACTOR;
//    if (FTL_IMPLEMENTATION == IMPL_DWF || FTL_IMPLEMENTATION == IMPL_HCWF ||
//        FTL_IMPLEMENTATION == IMPL_COLD) {
//        sstr << "-f" << std::setprecision(3) << HOT_FRACTION;
//    }
//    sstr << "-p" << std::setprecision(3) << WLVL_ACTIVATION_PROBABILITY;
//    sstr << "-" << traceID;
//    std::string fileName = sstr.str();
//    return fileName;
//}
//
//int
//main(int argc, char* argv[])
//{
//    load_config();
//
//    /// Override with command line settings
//
//    const std::string fileName = setup(argc, argv);
//
//    const std::string traceID = TRACEFILE.substr(0, 4);
//    EventReader evtRdr(TRACEFILE, NUMREQUESTS, READER_MODE,
//                       create_oracle_filename(traceID, HOT_FRACTION, 1));
//
//    std::vector<IOEvent> events = evtRdr.read_IO_events_from_trace(TRACEFILE);
//    const ulong maxLPN = evtRdr.find_max_lpn(events);
//    const ulong numLPN = maxLPN + 1;
//
//    PLANE_SIZE = static_cast<uint>(
//      std::ceil(static_cast<double>(numLPN) /
//                static_cast<double>(BLOCK_SIZE * (1.0 - SPARE_FACTOR))));
//
//    for (uint run = STARTRUN; run < (STARTRUN + NRUNS); run++) {
//        RandNrGen::reset();
//        HotColdID* hcID = new Static_HCID(maxLPN);
//
//        Ssd ssd(SSD_SIZE, hcID);
//        // ssd.initialize(uniqueLPNs);
//        ssd.initialize(numLPN);
//
//        const Controller& ctrl = ssd.get_controller();
//        ulong it = 0;
//        double startTime = 0.0;
//        while (ctrl.stats.get_currentPE() > MAXPE) {
//            if (BLOCK_ERASES - ctrl.stats.get_currentPE() % 100 == 0) {
//                std::cout << "CURRENT PE"
//                          << BLOCK_ERASES - ctrl.stats.get_currentPE()
//                          << std::endl;
//            }
//            const IOEvent& evt = events[it];
//            // Timings don't really matter for PE fairness/SSD endurance
//            ssd.event_arrive(evt.type, evt.lpn, evt.size, startTime);
//
//            it = (it + 1) % NUMREQUESTS;
//            startTime +=
//              5 * ((evt.type == READ) ? PAGE_READ_DELAY : PAGE_WRITE_DELAY);
//        }
//        ssd.write_statistics_csv(fileName, run);
//        delete hcID;
//    }
//    return 0;
//}
