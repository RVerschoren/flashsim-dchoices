/* Simulations with synthetic data using H/C swap wear leveling
 * Robin Verschoren 2019
 */

#include "ssd.h"
#include "ssd_run.h"

using namespace ssd;

int main(int argc, char* argv[])
{
    load_config();
    const SSD_Run_Params params = setup_synth(static_cast<uint>(argc), argv, WL_HOTCOLDGCSWAP_ERASE);
    run_synth(params);
    return 0;
}
