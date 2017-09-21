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
    FTL_IMPLEMENTATION = 6;
    DCHOICES_D = 5;
    print_config(NULL);
    printf("\n");

    Ssd ssd;

    const uint maxLBA= std::floor((1.0-SPARE_FACTOR)*BLOCK_SIZE*PLANE_SIZE*DIE_SIZE*PACKAGE_SIZE*SSD_SIZE);
    std::cout << maxLBA << std::endl;
    // Test one write to some blocks.
    for (uint i = 0; i < 3*maxLBA; i++)
    {
        if(i % 100 == 0) std::cout << "ITERATION : " << i << std::endl;
        /* event_arrive(event_type, logical_address, size, start_time) */
        double result = ssd.event_arrive(WRITE, i%maxLBA, 1, (double) 1+(2500*i));
    }

    ssd.print_statistics();
    return 0;
}
