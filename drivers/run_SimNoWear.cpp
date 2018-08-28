/* Trace simulations with DWF
 * Robin Verschoren 2017
 */

#include "ssd.h"
#include <iostream>
#include "ssd_run.h"

using namespace ssd;
//*
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
//    WEAR_LEVELER = WL_NONE;
//
//    // DChoices
//    GC_ALGORITHM = DCHOICES;
//
//    std::stringstream sstr;
//    sstr << std::fixed; // Print trailing zeroes
//    sstr << ftlstr;
//    sstr << "-b" << BLOCK_SIZE;
//    sstr << "-d" << DCHOICES_D;
//    sstr << "-sf" << std::setprecision(2) << SPARE_FACTOR;
//    if (FTL_IMPLEMENTATION == IMPL_DWF || FTL_IMPLEMENTATION == IMPL_HCWF ||
//        FTL_IMPLEMENTATION == IMPL_COLD) {
//        sstr << "-f" << std::setprecision(3) << HOT_FRACTION;
//        sstr << "-r" << std::setprecision(3) << HOT_REQUEST_RATIO;
//    }
//    std::string fileName = sstr.str();
//    return fileName;
//}

int
main(int argc, char* argv[])
{
    load_config();

    /// Override with command line settings
    const SSD_Run_Params params = setup_synth(argc, argv);
    ssd::run_synth(params);
    return 0;
}
