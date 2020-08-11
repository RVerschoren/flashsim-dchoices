/* Simulations with synthetic data
 * Robin Verschoren 2019
 */

#include "ssd.h"
#include "ssd_run.h"
#include <iostream>

using namespace ssd;

int main(int argc, char* argv[])
{
	load_config();
	const SSD_Run_Params params = setup_synth(argc, argv);
	///@TODO Enable setup_synth_new again
	///const SSD_Run_Params params = setup_synth_new(argc, argv);
	std::cout << "Simulation parameters:" << std::endl;
	std::cout << params.max_PE << std::endl;
	std::cout << params.max_cold_LPN << std::endl;
	std::cout << params.max_hot_LPN << std::endl;
	std::cout << params.nr_LPN << std::endl;
	std::cout << params.nr_events << std::endl;
	std::cout << params.nr_runs << std::endl;
	std::cout << params.out_filename << std::endl;
	std::cout << params.run_start << std::endl;

	// run_synth(params);
	return 0;
}
