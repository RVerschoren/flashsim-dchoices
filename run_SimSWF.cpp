/* Test to see if the drive writes and reads data correctly.
 * Robin Verschoren 2017
 */

#include "ssd.h"
//#include <stdio.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <sys/mman.h>
//#include <sys/types.h>
//#include <string.h>
//#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
//#include <cstdlib>

using namespace ssd;

int main(int argc, char *argv[])
{
    load_config();
    /// Override with command line settings
    BLOCK_SIZE = std::atoi(argv[1]);
    DCHOICES_D = std::atoi(argv[2]);
    SPARE_FACTOR = 1.0 - std::atof(argv[3]);
    PLANE_SIZE = std::atoi(argv[4]);
    const ulong maxPE = std::atoi(argv[5]);
    BLOCK_ERASES = 10000000;
    //SWF
    FTL_IMPLEMENTATION = 5;
    GC_ALGORITHM = 3;

    print_config(NULL);
    printf("\n");

    const uint maxLPN= std::floor((1.0-SPARE_FACTOR)*BLOCK_SIZE*PLANE_SIZE*DIE_SIZE*PACKAGE_SIZE*SSD_SIZE);

    //Example format filename : 'swf-b',I2,'-d',I3,'-rho',F4.2,'-r',F5.3,'-f',F5.3,'-WA.',I2,'.csv';
    std::stringstream sstr;
    sstr << std::fixed;//Print trailing zeroes
    sstr << "swf";
    sstr << "-b" << std::setw(2) << BLOCK_SIZE;
    sstr << "-d" << std::setw(3) << DCHOICES_D;
    sstr << "-rho" << std::setw(4) << std::setprecision(2) << std::atof(argv[3]);
    std::string fileName = sstr.str();

    RandNrGen::getInstance().reset();
    Ssd ssd;
    const Controller &ctrl = ssd.get_controller();
    uint i = 0;
    while(ctrl.stats.get_currentPE() <= maxPE)
    {
        const ulong lpn = RandNrGen::getInstance().get(maxLPN);
        ssd.event_arrive(WRITE, lpn, 1, (double) 1+(5000*i++)); //Timings don't really matter for PE fairness/SSD endurance
    }
    ssd.write_statistics_csv(fileName, 1);

    //std::cout << ctrl.stats.currentPE << std::endl;
    //while(ctrl.stats.get_currentPE() <= maxPE)
    //{
    //
    //    if(ctrl.stats.get_currentPE() % 1000 == 0){
    //        if(unreported)
    //        {
    //            std::cout << "PE CYCLES: " << ctrl.stats.currentPE << std::endl;
    //            unreported = false;
    //        }
    //    } else {
    //        unreported = true;
    //    }
    //    ssd.event_arrive(WRITE, i%maxLPN, 1, (double) 1+(2500*i));
    //    i++;
    //}
    //
    //ssd.print_statistics();
    return 0;
}
