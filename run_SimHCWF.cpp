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
    HOT_REQUEST_RATIO = std::atof(argv[5]);//r
    const uint startrun = std::atoi(argv[6]); //startrun
    const uint nruns = std::atoi(argv[7]); //Number of runs
    PLANE_SIZE = std::atoi(argv[8]);
    BLOCK_ERASES = 10000000;
    const ulong maxPE = BLOCK_ERASES - std::atoi(argv[9]); // Maximum number of PE cycles, counting back

    // HCWF
    FTL_IMPLEMENTATION = 7;
    // DChoices
    GC_ALGORITHM = 3;

    const ulong maxLPN = std::floor((1.0-SPARE_FACTOR)*BLOCK_SIZE*PLANE_SIZE*DIE_SIZE*PACKAGE_SIZE*SSD_SIZE);
    const ulong maxHotLPN = std::floor(HOT_FRACTION*maxLPN);
    const ulong maxColdLPN = maxLPN-maxHotLPN;

    //Example format filename : 'hcwf-b',I2,'-d',I3,'-rho',F4.2,'-r',F5.3,'-f',F5.3,'-WA.',I2,'.csv';
    std::stringstream sstr;
    sstr << std::fixed;//Print trailing zeroes
    sstr << "hcwf";
    sstr << "-b" << std::setw(2) << BLOCK_SIZE;
    sstr << "-d" << std::setw(3) << DCHOICES_D;
    sstr << "-rho" << std::setw(4) << std::setprecision(2) << std::atof(argv[3]);
    sstr << "-r" << std::setw(5) << std::setprecision(3) << HOT_REQUEST_RATIO;
    sstr << "-f" << std::setw(5) << std::setprecision(3) << HOT_FRACTION;
    std::string fileName = sstr.str();
    for(uint run = startrun; run < (startrun+nruns); run++)
    {
        RandNrGen::getInstance().reset();
        Ssd ssd;

        const Controller &ctrl = ssd.get_controller();
        uint i = 0;
        while(ctrl.stats.get_currentPE() > maxPE)
        {

            const ulong lpn = (RandNrGen::getInstance().get() <= HOT_REQUEST_RATIO)?
                                                RandNrGen::getInstance().get(maxHotLPN) : maxHotLPN+RandNrGen::getInstance().get(maxColdLPN);
            ssd.event_arrive(WRITE, lpn, 1, (double) 1+(2500*i++));//Timings don't really matter for PE fairness/SSD endurance
        }
        //ssd.print_statistics();
        ssd.write_statistics_csv(fileName, run);
    }
    return 0;
}
