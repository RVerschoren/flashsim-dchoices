/* Simulations with synthetic data
 * Robin Verschoren 2018
 */

#include "ssd.h"
#include "ssd_run.h"

using namespace ssd;

int
main(int argc, char* argv[])
{
    load_config();
    const SSD_Run_Params params = setup_synth(argc, argv);
    run_synth(params);
    return 0;
}
