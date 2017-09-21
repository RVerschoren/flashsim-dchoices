/* Synthetic data simulations with DWF
 * Robin Verschoren 2017
 */

#include "ssd.h"
#include "util.h"
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <random>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>

using namespace ssd;

int main(int argc, char *argv[])
{
    load_config();
    /// Override with command line settings
    BLOCK_SIZE = std::atoi(argv[1]);
    DCHOICES_D = std::atoi(argv[2]);
    SPARE_FACTOR = 1.0 - std::atof(argv[3]);
    HOT_FRACTION = std::atof(argv[4]);//f
    const uint startrun = std::atoi(argv[5]); //startrun
    const uint nruns = std::atoi(argv[6]); //Number of runs
    const ulong maxPE = std::atoi(argv[7]); // Maximum number of PE cycles
    const ulong numRequests = std::atol(argv[8]);
    const std::string traceFile(argv[9]);
    const std::string traceID = traceFile.substr(0,4);

    BLOCK_ERASES = 10000000;

    // DWF
    FTL_IMPLEMENTATION = 6;
    // DChoices
    GC_ALGORITHM = 3;
    std::vector<Event> events = read_event_from_trace(traceFile, &ssd::read_event_simple);

    const ulong maxLPN = std::floor((1.0-SPARE_FACTOR)*BLOCK_SIZE*PLANE_SIZE*DIE_SIZE*PACKAGE_SIZE*SSD_SIZE);
    const ulong maxHotLPN = std::floor(HOT_FRACTION*maxLPN);
    const ulong maxColdLPN = maxLPN-maxHotLPN;

    //Example format filename : 'dwf-b',I2,'-d',I3,'-rho',F4.2,'-r',F5.3,'-f',F5.3,'-WA.',I2,'.csv';
    std::stringstream sstr;
    sstr << std::fixed;//Print trailing zeroes
    sstr << "dwf";
    sstr << "-b" << std::setw(2) << BLOCK_SIZE;
    sstr << "-d" << std::setw(3) << DCHOICES_D;
    sstr << "-rho" << std::setw(4) << std::setprecision(2) << std::atof(argv[3]);
    sstr << "-r" << std::setw(5) << std::setprecision(3) << HOT_REQUEST_RATIO;
    sstr << "-f" << std::setw(5) << std::setprecision(3) << HOT_FRACTION;
    sstr << "-" << traceID;
    std::string fileName = sstr.str();
    for(uint run = startrun; run < (startrun+nruns); run ++ )
    {
        RandNrGen::getInstance().reset();
        Ssd ssd;

        const Controller &ctrl = ssd.get_controller();
        uint i = 0;
        while(ctrl.stats.get_currentPE() <= maxPE)
        {

            const ulong lpn = (RandNrGen::getInstance().get() <= HOT_REQUEST_RATIO)?
                                                RandNrGen::getInstance().get(maxHotLPN) : maxHotLPN+RandNrGen::getInstance().get(maxColdLPN);
            ssd.event_arrive(WRITE, lpn, 1, (double) 1+(2500*i++));//Timings don't really matter for PE fairness/SSD endurance
        }
        ssd.write_statistics_csv(fileName, run);
    }
    return 0;
}
