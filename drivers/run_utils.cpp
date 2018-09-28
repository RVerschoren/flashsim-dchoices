///TODO Put functions that read parameters for simulation runs here, maybe even code to run the simulations.

#include "ssd.h"
#include "ssd_run.h"
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <iostream>

using namespace ssd;

std::string
read_ftl(const std::string &ftl, int /*argc*/, char* argv[], ssd::uint &argit)
{
    std::string ftlstr;
    if (boost::iequals(ftl, "DWF")) {
        ftlstr = "dwf";
        FTL_IMPLEMENTATION = IMPL_DWF;
        HOT_FRACTION = std::stod(argv[argit++]);      // f
        HOT_REQUEST_RATIO = std::stof(argv[argit++]); // r
    } else if (boost::iequals(ftl, "HCWF")) {
        ftlstr = "hcwf";
        FTL_IMPLEMENTATION = IMPL_HCWF;
        HOT_FRACTION = std::stod(argv[argit++]);      // f
        HOT_REQUEST_RATIO = std::stof(argv[argit++]); // r
    } else if (boost::iequals(ftl, "COLD")) {
        ftlstr = "cold";
        FTL_IMPLEMENTATION = IMPL_COLD;
        HOT_FRACTION = std::stod(argv[argit++]);      // f
        HOT_REQUEST_RATIO = std::stof(argv[argit++]); // r
    } else if (boost::iequals(ftl, "STAT")) {
        ftlstr = "stat";
        FTL_IMPLEMENTATION = IMPL_STAT;
        HOT_FRACTION = std::stod(argv[argit++]);      // f
        HOT_REQUEST_RATIO = std::stof(argv[argit++]); // r
        HOT_SPARE_FACTOR_FRACTION = std::stof(argv[argit++]); // p
    } else {
        // SWF
        ftlstr = "swf";
        FTL_IMPLEMENTATION = IMPL_SWF;
        HOT_FRACTION = 0.0;
        HOT_REQUEST_RATIO = 0.0;
    }
    return ftlstr;
}

std::string
read_gc(const std::string &gca, int /*argc*/, char* argv[], ssd::uint &argit)
{
    std::string gcstr;
    if (boost::iequals(gca, "DCH")) {
        GC_ALGORITHM = GC_DCHOICES;
        DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
        gcstr = "d" + std::to_string(DCHOICES_D);
    } else if (boost::iequals(gca, "CAT")) {
        GC_ALGORITHM = GC_COSTAGETIME;
        gcstr = "cat";
    } else if (boost::iequals(gca, "CB")) {
        GC_ALGORITHM = GC_COSTBENEFIT;
        gcstr = "cb";
    } else if (boost::iequals(gca, "CBD")) {
        GC_ALGORITHM = GC_DCHOICES_COSTBENEFIT;
        DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
        gcstr = "cbd" + std::to_string(DCHOICES_D);
    } else if (boost::iequals(gca, "CATD")) {
        GC_ALGORITHM = GC_DCHOICES_COSTAGETIME;
        DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
        gcstr = "catd" + std::to_string(DCHOICES_D);
    } else if (boost::iequals(gca, "FIFO")) {
        GC_ALGORITHM = GC_FIFO;
        gcstr = "fifo";
    } else if (boost::iequals(gca, "RAND")){
        GC_ALGORITHM = GC_RANDOM;
        gcstr = "rand";
    } else {
        GC_ALGORITHM = GC_GREEDY;
        gcstr = "gr";
    }
    std::cout << gcstr << std::endl;
    return gcstr;
}

std::string
read_wlvl(const ssd::wl_scheme &wlvl, int argc, char* argv[], ssd::uint &argit)
{   std::string wlvl_str = "";
    if(wlvl != WL_NONE){
        WLVL_ACTIVATION_PROBABILITY = std::stod(argv[argit++]);
        std::stringstream sstr;
        if(wlvl == WL_BAN or wlvl == WL_BAN_PROB){
            WLVL_BAN_D = (argit <= (argc-1))? std::stoi(argv[argit++]) : 1;
            sstr << "ban" << WLVL_BAN_D;
        }else if(wlvl == WL_RANDOMSWAP){
            sstr << "swap";
        }else if(wlvl == WL_HOTCOLDSWAP){
            sstr << "hcswp";
        }else if(wlvl ==  WL_MAXVALID){
            sstr << "maxval";
        }
        wlvl_str = sstr.str();
    }
    WEAR_LEVELER = wlvl;
    return wlvl_str;
}


std::string
create_output_filename(const std::string &ftlstr, const std::string &gcstr, const std::string &wlvlstr, const std::string traceID = "")
{
    const bool is_trace = not traceID.empty();
    std::stringstream sstr;
    sstr << std::fixed; // Print trailing zeroes
    sstr << ftlstr;
    if(is_trace) sstr << "trace";
    sstr << wlvlstr;
    sstr << "-b" << BLOCK_SIZE;
    sstr << "-" << gcstr;
    sstr << "-sf" << std::setprecision(2) << SPARE_FACTOR;
    if (FTL_IMPLEMENTATION == IMPL_DWF || FTL_IMPLEMENTATION == IMPL_HCWF ||
        FTL_IMPLEMENTATION == IMPL_COLD) {
        sstr << "-f" << std::setprecision(3) << HOT_FRACTION;
        if(not is_trace) sstr << "-r" << std::setprecision(3) << HOT_REQUEST_RATIO;
    }
    if(WEAR_LEVELER != WL_NONE){
        sstr << "-p" << std::setprecision(3) << WLVL_ACTIVATION_PROBABILITY;
    }
    if(is_trace) sstr << "-" << traceID;
    return sstr.str();
}

SSD_Run_Params
ssd::setup_synth(int argc, char* argv[], ssd::wl_scheme wlvl)
{
    SSD_Run_Params params;

    uint argit = 1;
    const std::string ftl(argv[argit++]);
    BLOCK_SIZE = static_cast<uint>(std::stoi(argv[argit++]));
    const std::string gcstr = read_gc(argv[argit++], argc, argv, argit); //DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
    SPARE_FACTOR = std::stod(argv[argit++]);

    const std::string ftlstr = read_ftl(ftl, argc, argv, argit);

    params.run_start = static_cast<uint>(std::stoi(argv[argit++])); // startrun
    params.nr_runs = static_cast<uint>(std::stoi(argv[argit++]));    // Number of runs
    PLANE_SIZE = std::stoi(argv[argit++]);
    BLOCK_ERASES = 1000000;
    // Maximum number of PE cycles, counting back to leave a margin
    params.max_PE = BLOCK_ERASES -
            static_cast<uint>(std::stoi(argv[argit++]));

    const std::string wlvlstr = read_wlvl(wlvl, argc, argv, argit);

    params.out_filename = create_output_filename(ftlstr, gcstr, wlvlstr);
    std::cout << params.out_filename << std::endl;

    params.nr_LPN = static_cast<ulong>(
      std::floor((1.0 - SPARE_FACTOR) * BLOCK_SIZE * PLANE_SIZE * DIE_SIZE * PACKAGE_SIZE * SSD_SIZE));
    params.max_hot_LPN = static_cast<ulong>(std::floor(HOT_FRACTION * params.nr_LPN ));
    params.max_cold_LPN = params.nr_LPN - params.max_hot_LPN;

    return params;
}

void
ssd::run_synth(const SSD_Run_Params& params){
    for (uint run = params.run_start; run < (params.run_start + params.nr_runs); run++) {
        RandNrGen::reset();
        HotColdID* hcID = new Static_HCID(params.nr_LPN, HOT_FRACTION);

        Ssd ssd(SSD_SIZE, hcID);
        ssd.initialize(params.nr_LPN);

        const Controller& ctrl = ssd.get_controller();
        double time = 0.0;
        while (ctrl.stats.get_currentPE() > params.max_PE) {
            const ulong lpn = (RandNrGen::get() <= HOT_REQUEST_RATIO)
                                ? RandNrGen::get(params.max_hot_LPN)
                                : params.max_hot_LPN + RandNrGen::get(params.max_cold_LPN);
            time += ssd.event_arrive(WRITE, lpn, 1, time);
        }
        std::cout << time << std::endl;
        ssd.write_statistics_csv(params.out_filename, run);
        delete hcID;
    }
}

SSD_Run_Params
ssd::setup_trace(int argc, char* argv[], ssd::wl_scheme wlvl){
    SSD_Run_Params params;
    uint argit = 1;
    const std::string ftl(argv[argit++]);
    BLOCK_SIZE = static_cast<uint>(std::stoi(argv[argit++]));
    std::string gcstr = read_gc(argv[argit++], argc, argv, argit); //DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
    SPARE_FACTOR = std::stod(argv[argit++]);

    std::string ftlstr = read_ftl(ftl, argc, argv, argit);

    params.run_start = static_cast<uint>(std::stoi(argv[argit++])); // startrun
    params.nr_runs = static_cast<uint>(std::stoi(argv[argit++]));    // Number of runs
    BLOCK_ERASES = 1000000;
    params.max_PE = BLOCK_ERASES -
            static_cast<uint>(std::stoi(
              argv[argit++])); // Maximum number of PE cycles, counting back
    params.nr_events = std::stoul(argv[argit++]);
    params.trace_file = argv[argit++];
    params.traceID = params.trace_file.substr(0, 4);

    switch (std::stoi(argv[argit++])) {
        case 0:
            params.read_mode = EVTRDR_SIMPLE;
            break;
        case 1:
            params.read_mode = EVTRDR_BIOTRACER;
            break;
        default:
            params.read_mode = EVTRDR_SIMPLE;
            break;
    }

    const std::string wlvlstr = read_wlvl(wlvl, argc, argv, argit);

/*  std::stringstream sstr;
    sstr << std::fixed; // Print trailing zeroes
    sstr << ftlstr << "trace" << wlvlstr;
    sstr << "-b" << BLOCK_SIZE;
    sstr << "-d" << DCHOICES_D;
    sstr << "-sf" << std::setprecision(2) << SPARE_FACTOR;
    if (FTL_IMPLEMENTATION == IMPL_DWF || FTL_IMPLEMENTATION == IMPL_HCWF ||
        FTL_IMPLEMENTATION == IMPL_COLD) {
        sstr << "-f" << std::setprecision(3) << HOT_FRACTION;
    }
    if(WEAR_LEVELER != WL_NONE){
        sstr << "-p" << std::setprecision(3) << WLVL_ACTIVATION_PROBABILITY;
    }
    sstr << "-" << params.traceID;
    params.out_filename = sstr.str();*/
    params.out_filename = create_output_filename(ftlstr, gcstr, wlvlstr, params.traceID);

///TODO Finish
    return params;
}

void
ssd::run_trace(const SSD_Run_Params& params){
    EventReader evtRdr(params.trace_file, params.nr_events, params.read_mode,
                       create_oracle_filename(params.traceID, HOT_FRACTION, 1));

    std::vector<IOEvent> events = evtRdr.read_IO_events_from_trace(params.trace_file);
    const ulong maxLPN = evtRdr.find_max_lpn(events);
    const ulong numLPN = maxLPN + 1;

    PLANE_SIZE = static_cast<uint>(
      std::ceil(static_cast<double>(numLPN) /
                static_cast<double>(BLOCK_SIZE * (1.0 - SPARE_FACTOR))));

    for (uint run = params.run_start; run < (params.run_start + params.nr_runs); run++) {
        RandNrGen::reset();
        HotColdID* hcID = new Static_HCID(maxLPN);

        Ssd ssd(SSD_SIZE, hcID);
        ssd.initialize(numLPN);

        const Controller& ctrl = ssd.get_controller();
        ulong it = 0;
        double time = 0.0;
        while (ctrl.stats.get_currentPE() > params.max_PE) {
            if (BLOCK_ERASES - ctrl.stats.get_currentPE() % 100 == 0) {
                std::cout << "CURRENT PE"
                          << BLOCK_ERASES - ctrl.stats.get_currentPE()
                          << std::endl;
            }
            const IOEvent& evt = events[it];
            time += ssd.event_arrive(evt.type, evt.lpn, evt.size, time);

            it = (it + 1) % params.nr_events;
        }
        ssd.write_statistics_csv(params.out_filename, run);
        delete hcID;
    }
}
