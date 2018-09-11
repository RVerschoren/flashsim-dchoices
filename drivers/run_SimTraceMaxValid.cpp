/* Simulations with trace data using max valid wear leveling
 * Robin Verschoren 2018
 */

#include "ssd.h"
#include "ssd_run.h"

using namespace ssd;

int
main(int argc, char* argv[])
{
    load_config();
    const SSD_Run_Params params = setup_trace(argc, argv, WL_MAXVALID);
    run_trace(params);
    return 0;
}
