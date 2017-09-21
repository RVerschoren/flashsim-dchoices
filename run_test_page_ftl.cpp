/* Test to see if drive write and read data correct.
 * Matias Bjørling 24/2-2011
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

using namespace ssd;
#define SIZE 130
#define BLOCK_SIZE 64
#define NUM_BLOCKS 10000

typedef std::mt19937 MyRNG;  // the Mersenne Twister with a popular choice of parameters
uint32_t seed_val = 3268943513UL;           // populate somehow

using namespace ssd;

int main()
{
	load_config();
	print_config(NULL);
   //printf("Press ENTER to continue...");
   //getchar();
   printf("\n");

	MyRNG rng;
	rng.seed(seed_val);
	//std::uniform_int_distribution<uint32_t> uint_dist(0,10); // range [0,10]
	std::uniform_int_distribution<uint32_t> uint_dist(0,NUM_BLOCKS*BLOCK_SIZE-1);
	
	Ssd *ssd = new Ssd();

	double result;

	// Test one write to some blocks.
	for (int i = 0; i < SIZE; i++)
	{
		/* event_arrive(event_type, logical_address, size, start_time) */
		result = ssd -> event_arrive(WRITE, i*500, 1, (double) 1+(2500*i));

		//printf("Write time: %.20lf\n", result);
//		result = ssd -> event_arrive(WRITE, i+10240, 1, (double) 1);

	}
	
	//for (int j = 0; j < 3.0/SIZE*640000; j++)
	//	{
	//	for (int i = 0; i < SIZE; i++)
	//	{
	//		/* event_arrive(event_type, logical_address, size, start_time) */
	//		result = ssd -> event_arrive(WRITE, i*250, 1, (double) 1+(2500*i));
	//	}
	//	//printf("Write time: %.20lf\n", result);
	//}
	for (int j = 0; j < 10; j++)
	{
		for (int i = 0; i < BLOCK_SIZE*NUM_BLOCKS; i++)//FDW
		{
			uint addr = uint_dist(rng);
			/* event_arrive(event_type, logical_address, size, start_time) */
			result = ssd -> event_arrive(WRITE, addr, 1, (double) 1+(2500*i));
		}
		//printf("Write time: %.20lf\n", result);
	}
//	for (int i = 0; i < SIZE; i++)
//	{
//		/* event_arrive(event_type, logical_address, size, start_time) */
//		result = ssd -> event_arrive(READ, i*100000, 1, (double) 1+(500*i));
//		printf("Read time : %.20lf\n", result);
////		result = ssd -> event_arrive(READ, i, 1, (double) 1);
////		printf("Read time : %.20lf\n", result);
//	}

//	// Test writes and read to same block.
//	for (int i = 0; i < SIZE; i++)
//	{
//		result = ssd -> event_arrive(WRITE, i%64, 1, (double) 1+(250*i));
//
//		printf("Write time: %.20lf\n", result);
//	}
//	for (int i = 0; i < SIZE; i++)
//	{
//		result = ssd -> event_arrive(READ, i%64, 1, (double) 1+(500*i));
//		printf("Read time : %.20lf\n", result);
//	}

//	// Test random writes to a block
//	result = ssd -> event_arrive(WRITE, 5, 1, (double) 0.0);
//	printf("Write time: %.20lf\n", result);
//	result = ssd -> event_arrive(WRITE, 4, 1, (double) 300.0);
//	printf("Write time: %.20lf\n", result);
//	result = ssd -> event_arrive(WRITE, 3, 1, (double) 600.0);
//	printf("Write time: %.20lf\n", result);
//	result = ssd -> event_arrive(WRITE, 2, 1, (double) 900.0);
//	printf("Write time: %.20lf\n", result);
//	result = ssd -> event_arrive(WRITE, 1, 1, (double) 1200.0);
//	printf("Write time: %.20lf\n", result);
//	result = ssd -> event_arrive(WRITE, 0, 1, (double) 1500.0);
//	printf("Write time: %.20lf\n", result);
//
//	for (int i = 0; i < SIZE-6; i++)
//	{
//		/* event_arrive(event_type, logical_address, size, start_time) */
//		result = ssd -> event_arrive(WRITE, 6+i, 1, (double) 1800+(300*i));
//		printf("Write time: %.20lf\n", result);
//	}
//
//	// Force Merge
////	result = ssd -> event_arrive(WRITE, 10 , 1, (double) 0.0);
////	printf("Write time: %.20lf\n", result);

	ssd->print_statistics();
	delete ssd;
	return 0;
}
