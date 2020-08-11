#include "ssd.h"
#include "ssd_run.h"
#include <iomanip>
#include <iostream>
#include <string>

using namespace ssd;
using strings = std::vector<std::string>;

/**
 * Reduces a file path to the basename of the file.
 *
 */
std::string basename(const std::string& filepath)
{
	const std::string filename = filepath.substr(filepath.find_last_of("/\\") + 1);
	return filename.substr(0, filename.find_last_of('.'));
}

/**
 * @brief iequals Case insensitive string equality
 * @param str1 First string
 * @param str2 Second string
 * @return True if contents of both arguments are equal in lower case.
 */
bool iequals(const std::string& str1, const std::string& str2)
{
	uint length = str1.size();
	if (str2.size() != length) {
		return false;
	}

	for (uint i = 0; i < length; i++) {
		if (std::tolower(str1[i]) != std::tolower(str2[i]))
			return false;
	}
	return true;
}

std::string read_ftl(const std::string& ftl, const strings& argv, ssd::uint& argit)
{
	std::string ftlstr;
	if (iequals(ftl, "DWF")) {
		ftlstr = "dwf";
		FTL_IMPLEMENTATION = IMPL_DWF;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
	} else if (iequals(ftl, "DWFWDIFF")) {
		FTL_IMPLEMENTATION = IMPL_DWFWDIFF;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		HCSWAPWF_D = std::stoi(argv[argit++]);		  // d*
        MAX_ERASE_DIFF = std::stoi(argv[argit++]);	  // Wdiff
		std::stringstream sstr;
		sstr << std::setprecision(3) << "dwfwdiff";
		sstr << "d";
		if (HCSWAPWF_D == 0)
			sstr << "greedy";
		else
			sstr << HCSWAPWF_D;
		sstr << "wd" << MAX_ERASE_DIFF;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "HCWF")) {
		ftlstr = "hcwf";
		FTL_IMPLEMENTATION = IMPL_HCWF;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
	} else if (iequals(ftl, "HCWFFALSE")) {
		FTL_IMPLEMENTATION = IMPL_HCWF_FALSE;
		HOT_FRACTION = std::stod(argv[argit++]);		 // f
        HOT_REQUEST_RATIO = std::stod(argv[argit++]);	 // r
		HCID_FALSE_POS_RATIO = std::stod(argv[argit++]); // fh
		HCID_FALSE_NEG_RATIO = std::stod(argv[argit++]); // fc
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcwffp" << HCID_FALSE_POS_RATIO << "fn" << HCID_FALSE_NEG_RATIO;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "HCWFFIXEDFALSE")) {
		FTL_IMPLEMENTATION = IMPL_HCWF_FIXED_FALSE;
		HOT_FRACTION = std::stod(argv[argit++]);		 // f
        HOT_REQUEST_RATIO = std::stod(argv[argit++]);	 // r
		HCID_FALSE_POS_RATIO = std::stod(argv[argit++]); // fh
		HCID_FALSE_NEG_RATIO = std::stod(argv[argit++]); // fc
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcwffixedfp" << HCID_FALSE_POS_RATIO << "fn" << HCID_FALSE_NEG_RATIO;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "HCWFWDIFF")) {
		FTL_IMPLEMENTATION = IMPL_HCWFWDIFF;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		HCSWAPWF_D = std::stoi(argv[argit++]);		  // d*
        MAX_ERASE_DIFF = std::stoi(argv[argit++]);	  // Wdiff
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcwfwdiff";
		sstr << "d";
		if (HCSWAPWF_D == 0)
			sstr << "greedy";
		else
			sstr << HCSWAPWF_D;
		sstr << "wd" << MAX_ERASE_DIFF;
		ftlstr = sstr.str();

	} else if (iequals(ftl, "HCSWAPWF")) {
		FTL_IMPLEMENTATION = IMPL_HCSWAPWF;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		HCSWAPWF_D = std::stoi(argv[argit++]);		  // d*
		HCSWAPWF_PROBABILITY = std::stod(argv[argit++]);
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcswapwf";
		sstr << "d";
		if (HCSWAPWF_D == 0)
			sstr << "greedy";
		else
			sstr << HCSWAPWF_D;
		sstr << "-p" << std::setprecision(3) << HCSWAPWF_PROBABILITY;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "HCSWAPWF_HCTIE")) {
		FTL_IMPLEMENTATION = IMPL_HCSWAPWF_HOTCOLDTIE;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		HCSWAPWF_D = std::stoi(argv[argit++]);		  // d*
		HCSWAPWF_PROBABILITY = std::stod(argv[argit++]);
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcswapwfhctie";
		sstr << "d";
		if (HCSWAPWF_D == 0)
			sstr << "greedy";
		else
			sstr << HCSWAPWF_D;
		sstr << "-p" << std::setprecision(3) << HCSWAPWF_PROBABILITY;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "HCWFPLUSSWAP")) {
		FTL_IMPLEMENTATION = IMPL_HCWFPLUSSWAP;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		HCSWAPWF_D = std::stoi(argv[argit++]);		  // d*
		HCSWAPWF_PROBABILITY = std::stod(argv[argit++]);
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcwfplusswap";
		sstr << "d";
		if (HCSWAPWF_D == 0)
			sstr << "greedy";
		else
			sstr << HCSWAPWF_D;
		sstr << "-p" << std::setprecision(3) << HCSWAPWF_PROBABILITY;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "HCSWAPWFTERASE")) {
		FTL_IMPLEMENTATION = IMPL_HCSWAPWF_ERASETIE;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		HCSWAPWF_D = std::stoi(argv[argit++]);		  // d*
		HCSWAPWF_PROBABILITY = std::stod(argv[argit++]);
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcswapwfterase";
		sstr << "d";
		if (HCSWAPWF_D == 0)
			sstr << "greedy";
		else
			sstr << HCSWAPWF_D;
		sstr << "-p" << std::setprecision(3) << HCSWAPWF_PROBABILITY;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "HCSWAPWFERASE")) {
		FTL_IMPLEMENTATION = IMPL_HCSWAPWF_ERASE;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		HCSWAPWF_D = std::stoi(argv[argit++]);		  // d*
		HCSWAPWF_PROBABILITY = std::stod(argv[argit++]);
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcswapwferase";
		sstr << "d";
		if (HCSWAPWF_D == 0)
			sstr << "greedy";
		else
			sstr << HCSWAPWF_D;
		sstr << "-p" << std::setprecision(3) << HCSWAPWF_PROBABILITY;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "HCSWAPWFERASETVALID")) {
		FTL_IMPLEMENTATION = IMPL_HCSWAPWF_ERASE_VALIDTIE;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		HCSWAPWF_D = std::stoi(argv[argit++]);		  // d*
		HCSWAPWF_PROBABILITY = std::stod(argv[argit++]);
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcswapwferasetval";
		sstr << "d";
		if (HCSWAPWF_D == 0)
			sstr << "greedy";
		else
			sstr << HCSWAPWF_D;
		sstr << "-p" << std::setprecision(3) << HCSWAPWF_PROBABILITY;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "DSWAPWF")) {
		FTL_IMPLEMENTATION = IMPL_DSWAPWF;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		HCSWAPWF_D = std::stoi(argv[argit++]);
		HCSWAPWF_PROBABILITY = std::stod(argv[argit++]);
		std::stringstream sstr;
		sstr << std::setprecision(3) << "dswapwf";
		sstr << "d";
		if (HCSWAPWF_D == 0)
			sstr << "greedy";
		else
			sstr << HCSWAPWF_D;
		sstr << "-p" << std::setprecision(3) << HCSWAPWF_PROBABILITY;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "HCWF_DHC")) {
		FTL_IMPLEMENTATION = IMPL_HCWF_DHC;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		DCHOICES_DH = std::stoi(argv[argit++]);		  // dH
		DCHOICES_DC = std::stoi(argv[argit++]);		  // dC
		std::stringstream sstr;
		sstr << std::setprecision(3) << "hcwfdH";
		if (DCHOICES_DH == 0)
			sstr << "greedy";
		else
			sstr << DCHOICES_DH;
		sstr << "C";
		if (DCHOICES_DC == 0)
			sstr << "greedy";
		else
			sstr << DCHOICES_DC;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "COLD")) {
		ftlstr = "cold";
		FTL_IMPLEMENTATION = IMPL_COLD;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
	} else if (iequals(ftl, "COLD_DHC")) {
		FTL_IMPLEMENTATION = IMPL_COLD_DHC;
		HOT_FRACTION = std::stod(argv[argit++]);	  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]); // r
		DCHOICES_DH = std::stoi(argv[argit++]);		  // dH
		DCHOICES_DC = std::stoi(argv[argit++]);		  // dC
		std::stringstream sstr;
		sstr << std::setprecision(3) << "colddH" << DCHOICES_DH << "C" << DCHOICES_DC;
		ftlstr = sstr.str();
	} else if (iequals(ftl, "STAT")) {
		FTL_IMPLEMENTATION = IMPL_STAT;
		HOT_FRACTION = std::stod(argv[argit++]);			  // f
		HOT_REQUEST_RATIO = std::stod(argv[argit++]);		  // r
		HOT_SPARE_FACTOR_FRACTION = std::stod(argv[argit++]); // p
		std::stringstream sstr;
		sstr << "stat" << std::setprecision(3) << HOT_SPARE_FACTOR_FRACTION;
		ftlstr = sstr.str();
    } else if (iequals(ftl, "SWFWDIFF")) {
        FTL_IMPLEMENTATION = IMPL_SWFWDIFF;
        HOT_FRACTION = 0.50;
        HOT_REQUEST_RATIO = 0.50;
        HCSWAPWF_D = std::stoi(argv[argit++]);	   // d*
        MAX_ERASE_DIFF = std::stoi(argv[argit++]); // Wdiff
        std::stringstream sstr;
        sstr << std::setprecision(3) << "swfwdiff";
        sstr << "d";
        if (HCSWAPWF_D == 0)
            sstr << "greedy";
        else
            sstr << HCSWAPWF_D;
        sstr << "wd" << MAX_ERASE_DIFF;
        ftlstr = sstr.str();
    } else {
		// SWF
		ftlstr = "swf";
		FTL_IMPLEMENTATION = IMPL_SWF;
        HOT_FRACTION = 0.50;
        HOT_REQUEST_RATIO = 0.50;
	}
	return ftlstr;
}

std::string read_gc(const std::string& gca, const strings& argv, ssd::uint& argit)
{
	std::string gcstr;
	if (iequals(gca, "DCH")) {
		GC_ALGORITHM = GC_DCHOICES;
		DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
		gcstr = "d" + std::to_string(DCHOICES_D);
	} else if (iequals(gca, "DCH_HOTCOLD")) {
		GC_ALGORITHM = GC_DCH_HOTCOLD;
		DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
		gcstr = "dhotcold" + std::to_string(DCHOICES_D);
	} else if (iequals(gca, "DCH_TIEERASE")) {
		GC_ALGORITHM = GC_DCHOICES_ERASETIE;
		DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
		gcstr = "dtieerase" + std::to_string(DCHOICES_D);
	} else if (iequals(gca, "DCH_ERASE")) {
		GC_ALGORITHM = GC_DCHOICES_ERASE;
		DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
		gcstr = "derase" + std::to_string(DCHOICES_D);
	} else if (iequals(gca, "DCH_ERASE_TIEVALID")) {
		GC_ALGORITHM = GC_DCHOICES_ERASE_VALIDTIE;
		DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
		gcstr = "derasetievalid" + std::to_string(DCHOICES_D);
	} else if (iequals(gca, "CAT")) {
		GC_ALGORITHM = GC_COSTAGETIME;
		gcstr = "cat";
	} else if (iequals(gca, "CB")) {
		GC_ALGORITHM = GC_COSTBENEFIT;
		gcstr = "cb";
	} else if (iequals(gca, "CBD")) {
		GC_ALGORITHM = GC_DCHOICES_COSTBENEFIT;
		DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
		gcstr = "cbd" + std::to_string(DCHOICES_D);
	} else if (iequals(gca, "CATD")) {
		GC_ALGORITHM = GC_DCHOICES_COSTAGETIME;
		DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
		gcstr = "catd" + std::to_string(DCHOICES_D);
	} else if (iequals(gca, "FIFO")) {
		GC_ALGORITHM = GC_FIFO;
		gcstr = "fifo";
	} else if (iequals(gca, "RAND")) {
		GC_ALGORITHM = GC_RANDOM;
		gcstr = "rand";
	} else {
		GC_ALGORITHM = GC_GREEDY;
		gcstr = "gr";
	}
	std::cout << gcstr << std::endl;
	return gcstr;
}

std::string read_wlvl_parameters(const ssd::wl_scheme& wlvl, const strings& argv, ssd::uint& argit,
								 const double blockSize, const double spareFactor, const double blockSwapCost,
								 const bool verbose = true)
{
	const uint argc = argv.size();
	std::string wlvl_str;
	if (wlvl != WL_NONE) {
		std::stringstream sstr;
		sstr << std::fixed; // Print trailing zeroes
		if (wlvl == WL_HOTCOLDSWAP_RELATIVE) {
			const double relativeIncreaseWA = std::stod(argv[argit++]);
			sstr << "hcswprelinc" << std::setprecision(3) << relativeIncreaseWA;
			WLVL_ACTIVATION_PROBABILITY = (relativeIncreaseWA)*blockSize / (blockSize * spareFactor * blockSwapCost);
		} else {
			WLVL_ACTIVATION_PROBABILITY = std::stod(argv[argit++]);

			if (wlvl == WL_BAN or wlvl == WL_BAN_PROB) {
				WLVL_BAN_D = (argit <= (argc - 1)) ? std::stoi(argv[argit++]) : 1;
				sstr << "ban" << WLVL_BAN_D;
			} else if (wlvl == WL_RANDOMSWAP) {
				sstr << "swap";
			} else if (wlvl == WL_HOTCOLDSWAP) {
				sstr << "hcswp";
			} else if (wlvl == WL_HOTCOLDGCSWAP) {
				sstr << "hcgcswp";
			} else if (wlvl == WL_HOTCOLDGCSWAP_ERASE) {
				sstr << "hcgcswperase";
			} else if (wlvl == WL_MAXVALID) {
				sstr << "maxval";
			}
		}
		wlvl_str = sstr.str();
	}
	WEAR_LEVELER = wlvl;
	if (verbose)
		std::cout << wlvl_str << std::endl;
	return wlvl_str;
}

std::string create_output_filename(const std::string& ftlstr, const std::string& gcstr, const std::string& wlvlstr,
								   const std::string& traceID = "")
{
	const bool is_trace = not traceID.empty();
	std::stringstream sstr;
	sstr << std::fixed; // Print trailing zeroes
	sstr << ftlstr;
	if (is_trace)
		sstr << "trace";
	sstr << wlvlstr;
	sstr << "-b" << BLOCK_SIZE;
	sstr << "-" << gcstr;
	sstr << "-sf" << std::setprecision(2) << SPARE_FACTOR;
	if (FTL_IMPLEMENTATION == IMPL_DWF || FTL_IMPLEMENTATION == IMPL_DWFWDIFF || FTL_IMPLEMENTATION == IMPL_HCWF ||
		FTL_IMPLEMENTATION == IMPL_COLD || FTL_IMPLEMENTATION == IMPL_HCWF_FALSE ||
		FTL_IMPLEMENTATION == IMPL_HCWF_FIXED_FALSE || FTL_IMPLEMENTATION == IMPL_HCWFWDIFF ||
		FTL_IMPLEMENTATION == IMPL_HCSWAPWF || FTL_IMPLEMENTATION == IMPL_HCWFPLUSSWAP ||
		FTL_IMPLEMENTATION == IMPL_HCSWAPWF_ERASETIE || FTL_IMPLEMENTATION == IMPL_HCSWAPWF_ERASE_VALIDTIE ||
		FTL_IMPLEMENTATION == IMPL_HCSWAPWF_ERASE || FTL_IMPLEMENTATION == IMPL_HCSWAPWF_HOTCOLDTIE ||
		FTL_IMPLEMENTATION == IMPL_DSWAPWF || FTL_IMPLEMENTATION == IMPL_HCWF_DHC ||
		FTL_IMPLEMENTATION == IMPL_COLD_DHC || FTL_IMPLEMENTATION == IMPL_STAT) {
		sstr << "-f" << std::setprecision(3) << HOT_FRACTION;
		if (not is_trace)
			sstr << "-r" << std::setprecision(3) << HOT_REQUEST_RATIO;
	}
	if (WEAR_LEVELER != WL_NONE) {
		sstr << "-p" << std::setprecision(3) << WLVL_ACTIVATION_PROBABILITY;
	}
	if (MAXPE_THRESHOLD > 1) {
		sstr << "-fPE" << std::setprecision(3)
			 << static_cast<double>(MAXPE_THRESHOLD) /
					static_cast<double>(PLANE_SIZE * DIE_SIZE * PACKAGE_SIZE * SSD_SIZE);
	}
	if (is_trace)
		sstr << "-" << traceID;
	return sstr.str();
}

SSD_Run_Params ssd::setup_synth(int argc, char* argv[], ssd::wl_scheme wlvl)
{
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++) {
		args.emplace_back(argv[i]);
	}
	return setup_synth(args, wlvl);
}

SSD_Run_Params ssd::setup_synth(const std::vector<std::string>& args, ssd::wl_scheme wlvl)
{
	SSD_Run_Params params;

	uint argit = 0;
	const std::string ftl(args.at(argit++));
	BLOCK_SIZE = static_cast<uint>(std::stoi(args.at(argit++)));
	const std::string gcstr = read_gc(args.at(argit++), args, argit);
	// DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
	SPARE_FACTOR = std::stod(args.at(argit++));

	const std::string ftlstr = read_ftl(ftl, args, argit);
	std::cout << ftlstr << std::endl;

	params.run_start = static_cast<uint>(std::stoi(args.at(argit++))); // startrun
	params.nr_runs = static_cast<uint>(std::stoi(args.at(argit++)));   // Number of runs
	PLANE_SIZE = static_cast<uint>(std::ceil(std::stod(args.at(argit++)) / (SSD_SIZE * PACKAGE_SIZE * DIE_SIZE)));
	BLOCK_ERASES = 1000000;
	// Maximum number of PE cycles, counting back to leave a margin
	params.max_PE = BLOCK_ERASES - static_cast<ulong>(std::stoi(args.at(argit++)));
	///@TODO Cleanup max_PE
	MAX_PE = params.max_PE;

	const std::string wlvlstr =
		read_wlvl_parameters(wlvl, args, argit, BLOCK_SIZE, SPARE_FACTOR, 2 * BLOCK_SIZE * (1.0 - SPARE_FACTOR));

	std::cout << argit << "  out of  " << args.size() << std::endl;
	if (argit < args.size())
		MAXPE_THRESHOLD = static_cast<ulong>(
			std::ceil(std::stod(args.at(argit++)) * PLANE_SIZE * DIE_SIZE * PACKAGE_SIZE * SSD_SIZE));
	std::cout << "Threshold number of blocks over MAXPE: " << MAXPE_THRESHOLD << std::endl;

	std::stringstream filenamestr;
	filenamestr << create_output_filename(ftlstr, gcstr, wlvlstr); /// NOTE  << "-MAXPE" << (BLOCK_ERASES - MAX_PE);

	params.out_filename = filenamestr.str();
	std::cout << params.out_filename << std::endl;

	params.nr_LPN = static_cast<ulong>(
		std::floor((1.0 - SPARE_FACTOR) * BLOCK_SIZE * PLANE_SIZE * DIE_SIZE * PACKAGE_SIZE * SSD_SIZE));
	params.max_hot_LPN = static_cast<ulong>(std::floor(HOT_FRACTION * params.nr_LPN));
	params.max_cold_LPN = params.nr_LPN - params.max_hot_LPN;

	return params;
}

void ssd::run_synth(const SSD_Run_Params& params)
{
	double sumWA = 0.0;
	for (uint run = params.run_start; run < (params.run_start + params.nr_runs); run++) {
		RandNrGen::reset();

		HotColdID* hcID = new Static_HCID(params.nr_LPN, HOT_FRACTION);
		/*HotColdID* hcID;
		if(FTL_IMPLEMENTATION == IMPL_HCWF_FALSE){
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																		hcID = new
		Static_False_HCID(params.nr_LPN, HCID_FALSE_POS_RATIO, HCID_FALSE_NEG_RATIO, HOT_FRACTION);
		}else{ hcID = new Static_HCID(params.nr_LPN, HOT_FRACTION);
		}*/

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
		sumWA += (double)(ctrl.stats.numGCWrite + ctrl.stats.numFTLWrite + ctrl.stats.numWLWrite) /
				 (double)ctrl.stats.numFTLWrite;
		delete hcID;
	}
	std::cout << "Mean WA: " << sumWA / params.nr_runs << std::endl;
}

void add_values(std::vector<double>& out, const std::vector<uint>& in)
{
	/// BUG Assumes in is as big as out vector
	if (out.size() == 0) {
		// out.reserve(in.size());
		// out.assign(in.size(),0.0);
		for (uint i = 0; i < in.size(); i++) {
			out.push_back(in[i]);
		}
	} else {
		for (uint i = 0; i < in.size(); i++) {
			out[i] += in[i];
		}
	}
}

void add_values(std::vector<double>& out, const std::vector<double>& in)
{
	/// BUG Assumes in is as big as out vector
	if (out.size() == 0) {
		// out.reserve(in.size());
		// out.assign(in.size(),0.0);
		for (uint i = 0; i < in.size(); i++) {
			out.push_back(in[i]);
		}
	} else {
		for (uint i = 0; i < in.size(); i++) {
			out[i] += in[i];
		}
	}
}

void mean_values(std::vector<double>& vector, uint numberRuns)
{
	for (double& d : vector) {
		d /= numberRuns;
	}
}

void ssd::run_synth(const SSD_Run_Params& params, double& WA, std::vector<double>& endurance,
					std::vector<double>& fairness, std::vector<double>& hotbl, std::vector<double>& coldbl)
{
	for (uint run = params.run_start; run < (params.run_start + params.nr_runs); run++) {
		RandNrGen::reset();

		HotColdID* hcID = new Static_HCID(params.nr_LPN, HOT_FRACTION);
		/*HotColdID* hcID;
		if(FTL_IMPLEMENTATION == IMPL_HCWF_FALSE){
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																   hcID = new
		Static_False_HCID(params.nr_LPN, HCID_FALSE_POS_RATIO, HCID_FALSE_NEG_RATIO, HOT_FRACTION);
		}else{ hcID = new Static_HCID(params.nr_LPN, HOT_FRACTION);
		}*/

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
		// std::cout << time << std::endl;
		// ssd.write_statistics_csv(params.out_filename, run);
		// sumWA += (double)(ctrl.stats.numGCWrite + ctrl.stats.numFTLWrite + ctrl.stats.numWLWrite)
		// /
		//         (double)ctrl.stats.numFTLWrite;
		Stats stats = ctrl.stats;
		WA += stats.getWA();
		/// NOTE Every vector stat starts at 0 for now
		add_values(endurance, stats.SSDendurance);
		// std::cout << "LAST ENDURANCE WAS " << stats.SSDendurance.size() << " WITH VALUE "<<
		// stats.SSDendurance[stats.SSDendurance.size()-1] << std::endl; std::cout << "AFTER ADDING
		// ENDURANCE WAS "
		// << endurance.size() << " WITH VALUE "<< endurance[endurance.size()-1] << std::endl;
		add_values(fairness, stats.PEfairness);
		add_values(hotbl, stats.hotBlocks);
		add_values(coldbl, stats.coldBlocks);
		delete hcID;
	}
	// std::cout << "Mean WA: " << sumWA / params.nr_runs << std::endl;
	// Mean over all runs
	WA = WA / params.nr_runs;
	mean_values(endurance, params.nr_runs);
	// std::cout << "MEAN LAST ENDURANCE WAS " << endurance[endurance.size()-1] <<  std::endl;
	mean_values(fairness, params.nr_runs);
	mean_values(hotbl, params.nr_runs);
	mean_values(coldbl, params.nr_runs);
	/// TODO DO the rest here
}

SSD_Run_Params ssd::setup_trace(int argc, char* argv[], ssd::wl_scheme wlvl)
{
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++) {
		args.emplace_back(argv[i]);
	}
	return setup_trace(args, wlvl);
}

SSD_Run_Params ssd::setup_trace(const std::vector<std::string>& args, ssd::wl_scheme wlvl)
{
	SSD_Run_Params params;
	uint argit = 0;
	const std::string ftl(args.at(argit++));
	BLOCK_SIZE = static_cast<uint>(std::stoi(args.at(argit++)));
	std::string gcstr = read_gc(args.at(argit++), args,
								argit); // DCHOICES_D = static_cast<uint>(std::stoi(argv[argit++]));
	SPARE_FACTOR = std::stod(args.at(argit++));

	// Reads HOT_FRACTION and HOT_REQUEST_RATIO but ignores HOT_REQUEST_RATIO
	std::string ftlstr = read_ftl(ftl, args, argit);

	params.run_start = static_cast<uint>(std::stoi(args.at(argit++))); // startrun
	params.nr_runs = static_cast<uint>(std::stoi(args.at(argit++)));   // Number of runs
	BLOCK_ERASES = 1000000;
	params.max_PE =
		BLOCK_ERASES - static_cast<uint>(std::stoi(args.at(argit++))); // Maximum number of PE cycles, counting back
																	   ///@TODO Cleanup max_PE
	MAX_PE = params.max_PE;
	params.nr_events = std::stoul(args.at(argit++));
	params.trace_file = args.at(argit++);
	const std::string trace_file_basename = basename(params.trace_file);
	params.traceID = trace_file_basename.substr(0, 4);

	switch (std::stoi(args.at(argit++))) {
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

	const std::string wlvlstr =
		read_wlvl_parameters(wlvl, args, argit, BLOCK_SIZE, SPARE_FACTOR, 2 * BLOCK_SIZE * (1.0 - SPARE_FACTOR));

	params.out_filename = create_output_filename(ftlstr, gcstr, wlvlstr, params.traceID);

	/// TODO Finish
	return params;
}

void ssd::run_trace(const SSD_Run_Params& params)
{
	EventReader evtRdr(params.trace_file, params.nr_events, params.read_mode,
					   create_oracle_filename(params.traceID, HOT_FRACTION, 1));

	std::vector<IOEvent> events = evtRdr.read_IO_events_from_trace(params.trace_file);
	const ulong maxLPN = evtRdr.find_max_lpn(events);
	const ulong numLPN = maxLPN + 1;

	const double nrBlocksNeeded = static_cast<double>(numLPN) / static_cast<double>(BLOCK_SIZE * (1.0 - SPARE_FACTOR));

	PLANE_SIZE = static_cast<uint>(std::ceil(nrBlocksNeeded / (SSD_SIZE * PACKAGE_SIZE * DIE_SIZE)));

	std::cout << "Writing results to " << params.out_filename << std::endl;
	for (uint run = params.run_start; run < (params.run_start + params.nr_runs); run++) {
		RandNrGen::reset();
		/// TODO Remove params.nr_LPN
		HotColdID* hcID = new Static_HCID(numLPN /*params.nr_LPN*/, HOT_FRACTION);
		/*HotColdID* hcID;
		if(FTL_IMPLEMENTATION == IMPL_HCWF_FALSE){
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																		hcID = new
		Static_False_HCID(params.nr_LPN, HCID_FALSE_POS_RATIO, HCID_FALSE_NEG_RATIO, HOT_FRACTION);
		}else{ hcID = new Static_HCID(params.nr_LPN, HOT_FRACTION);
		}*/

		Ssd ssd(SSD_SIZE, hcID);
		std::cout << "INIT FTL WITH " << numLPN << "LPNs on " << PLANE_SIZE << " blocks. Bon voyage!" << std::endl;
		ssd.initialize(numLPN);
		std::cout << "FTL INIT DONE IN RUN_TRACE" << std::endl;
		const Controller& ctrl = ssd.get_controller();
		ulong it = 0;
		double time = 0.0;
		while (ctrl.stats.get_currentPE() > params.max_PE) {
			if (BLOCK_ERASES - ctrl.stats.get_currentPE() % 100 == 0) {
				std::cout << "CURRENT PE" << BLOCK_ERASES - ctrl.stats.get_currentPE() << std::endl;
			}
			const IOEvent& evt = events[it];
			time += ssd.event_arrive(evt.type, evt.lpn, evt.size, time);

			it = (it + 1) % params.nr_events;
		}
		ssd.write_statistics_csv(params.out_filename, run);
		delete hcID;
	}
}
