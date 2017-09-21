/* Test to see if the drive writes and reads data correctly.
 * Robin Verschoren 2017
 */

#include "ssd.h"
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

using namespace ssd;

int main()
{
	load_config();
	print_config(NULL);
	//printf("Press ENTER to continue...");
	//getchar();
	printf("\n");

	//MyRNG rng;
	//uint32_t seed_val = 3268943513UL;           // populate somehow
	//rng.seed(seed_val);
	////std::uniform_int_distribution<uint32_t> uint_dist(0,10); // range [0,10]
	//std::uniform_int_distribution<uint32_t> uint_dist(0,NUM_BLOCKS*BLOCK_SIZE-1);
    FTL_IMPLEMENTATION = 5;
    DCHOICES_D = 5;
    PLANE_SIZE = 1000;
    Ssd ssd;

	const uint maxLBA= std::floor((1.0-SPARE_FACTOR)*BLOCK_SIZE*PLANE_SIZE*DIE_SIZE*PACKAGE_SIZE*SSD_SIZE);
    std::cout << maxLBA << std::endl;
	// Test one write to some blocks.
    for (uint i = 0; i < 5*maxLBA; i++)
    {
        if(i % 100 == 0) std::cout << "ITERATION : " << i << std::endl;
        double result = ssd.event_arrive(WRITE, i%maxLBA, 1, (double) 1+(2500*i));
    }
    //uint i = 0;
    //std::cout << ssd.get_controller().stats.currentPE << std::endl;
    //while(ssd.get_controller().stats.currentPE <= 5000)
    //{
    //
    //    if(ssd.get_controller().stats.currentPE % 100 == 0) std::cout << "PE CYCLES: " << ssd.get_controller().stats.currentPE << std::endl;
    //    //if(i % 100 == 0) std::cout << "ITERATION : " << i << std::endl;
    //	/* event_arrive(event_type, logical_address, size, start_time) */
    //    double result = ssd.event_arrive(WRITE, i%maxLBA, 1, (double) 1+(2500*i));
    //
    //	//ssd->print_statistics();
    //    i++;
    //    //std::cout << i << std::endl;
    //	//printf("Write time: %.20lf\n", result);
    //}

    ssd.print_statistics();
//	delete ssd;
	return 0;
}
