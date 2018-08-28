/* Trace simulations with DWF
 * Robin Verschoren 2017
 */

#include "ssd.h"
#include "ssd_run.h"

using namespace ssd;
//uint STARTRUN;
//uint NRUNS;
//ulong MAXPE;
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
//        HOT_FRACTION = std::stod(argv[argit++]);      // f
//        HOT_REQUEST_RATIO = std::atof(argv[argit++]); // r
//    } else if (boost::iequals(ftl, "HCWF")) {
//        ftlstr = "hcwf";
//        FTL_IMPLEMENTATION = IMPL_HCWF;
//        HOT_FRACTION = std::stod(argv[argit++]);      // f
//        HOT_REQUEST_RATIO = std::atof(argv[argit++]); // r
//    } else if (boost::iequals(ftl, "COLD")) {
//        ftlstr = "cold";
//        FTL_IMPLEMENTATION = IMPL_COLD;
//        HOT_FRACTION = std::stod(argv[argit++]);      // f
//        HOT_REQUEST_RATIO = std::atof(argv[argit++]); // r
//    } else {
//        // SWF
//        ftlstr = "swf";
//        FTL_IMPLEMENTATION = IMPL_SWF;
//        HOT_FRACTION = 0.0;
//        HOT_REQUEST_RATIO = 0.0;
//    }
//
//    STARTRUN = static_cast<uint>(std::stoi(argv[argit++])); // startrun
//    NRUNS = static_cast<uint>(std::stoi(argv[argit++]));    // Number of runs
//    PLANE_SIZE = std::atoi(argv[argit++]);
//    BLOCK_ERASES = 10000000;
//    MAXPE = BLOCK_ERASES -
//            static_cast<uint>(std::stoi(
//              argv[argit++])); // Maximum number of PE cycles, counting back
//
//    WLVL_ACTIVATION_PROBABILITY =
//      std::stod(argv[argit++]); // Ban's algorithm p parameter
//    std::cout << WLVL_ACTIVATION_PROBABILITY << std::endl;
//    WEAR_LEVELER = WL_RANDOMSWAP;
//
//    // DChoices
//    GC_ALGORITHM = DCHOICES;
//
//    std::stringstream sstr;
//    sstr << std::fixed; // Print trailing zeroes
//    sstr << ftlstr << "swap";
//    sstr << "-b" << BLOCK_SIZE;
//    sstr << "-d" << DCHOICES_D;
//    sstr << "-sf" << std::setprecision(2) << SPARE_FACTOR;
//    if (FTL_IMPLEMENTATION == IMPL_DWF || FTL_IMPLEMENTATION == IMPL_HCWF ||
//        FTL_IMPLEMENTATION == IMPL_COLD) {
//        sstr << "-f" << std::setprecision(3) << HOT_FRACTION;
//        sstr << "-r" << std::setprecision(3) << HOT_REQUEST_RATIO;
//    }
//    sstr << "-p" << std::setprecision(3) << WLVL_ACTIVATION_PROBABILITY;
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
//    const ulong numLPN = static_cast<ulong>(
//      std::floor((1.0 - SPARE_FACTOR) * BLOCK_SIZE * PLANE_SIZE * DIE_SIZE *
//                 PACKAGE_SIZE * SSD_SIZE));
//    const ulong maxHotLPN =
//      static_cast<ulong>(std::floor(HOT_FRACTION * numLPN));
//    const ulong maxColdLPN = numLPN - maxHotLPN;
//
//    for (uint run = STARTRUN; run < (STARTRUN + NRUNS); run++) {
//        RandNrGen::reset();
//        HotColdID* hcID = new Static_HCID(numLPN, HOT_FRACTION);
//
//        Ssd ssd(SSD_SIZE, hcID);
//        ssd.initialize(numLPN);
//
//        const Controller& ctrl = ssd.get_controller();
//        uint it = 0;
//        while (ctrl.stats.get_currentPE() > MAXPE) {
//            const ulong lpn = (RandNrGen::get() <= HOT_REQUEST_RATIO)
//                                ? RandNrGen::get(maxHotLPN)
//                                : maxHotLPN + RandNrGen::get(maxColdLPN);
//            // Timings don't really matter for PE fairness/SSD endurance
//            ssd.event_arrive(WRITE, lpn, 1, (double)1 + (2500 * it++));
//        }
//        ssd.write_statistics_csv(fileName, run);
//        delete hcID;
//    }
//    return 0;
//}

int
main(int argc, char* argv[])
{
    load_config();

    /// Override with command line settings
    const SSD_Run_Params params = setup_synth(argc, argv, "swap", WL_RANDOMSWAP);
    ssd::run_synth(params);
    return 0;
}
