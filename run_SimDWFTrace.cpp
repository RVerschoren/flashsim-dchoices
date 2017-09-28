/* Trace simulations with DWF
 * Robin Verschoren 2017
 */

#include "ssd.h"
#include <iostream>
#include <iomanip>

using namespace ssd;

int main(int argc, char *argv[])
{
    load_config();
    /// Override with command line settings
    BLOCK_SIZE = std::atoi(argv[1]);
    DCHOICES_D = std::atoi(argv[2]);
    SPARE_FACTOR = std::atof(argv[3]);
    HOT_FRACTION = std::atof(argv[4]);//f
    const uint startrun = std::atoi(argv[5]); //startrun
    const uint nruns = std::atoi(argv[6]); //Number of runs
    BLOCK_ERASES = 10000000;
    const ulong maxPE = BLOCK_ERASES - std::atoi(argv[7]); // Maximum number of PE cycles, counting back
    const ulong numRequests = std::atol(argv[8]);
    const std::string traceFile(argv[9]);
    const std::string traceID = traceFile.substr(0,4);

    std::function<ssd::Event (std::string)> readEvtFunc;
    switch(std::stoi(argv[10]))
    {
        case 0:
        readEvtFunc = ssd::read_event_simple;
        break;
        case 1:
        readEvtFunc = ssd::read_event_BIOtracer;
        break;
    default:
        readEvtFunc = ssd::read_event_simple;
        break;
    }

    uint nrFrames = 1;

    // DWF
    FTL_IMPLEMENTATION = 6;
    // DChoices
    GC_ALGORITHM = 3;

    std::vector<Event> events = read_event_from_trace(traceFile, readEvtFunc);
    const ulong numUniqueLPNs = count_unique_lpns(events);

    PLANE_SIZE = std::ceil( (double) numUniqueLPNs / (double) (BLOCK_SIZE * (1.0-SPARE_FACTOR)) );
    //Example oracle filename
    std::vector<bool> oracle = read_oracle( create_oracle_filename(traceID, HOT_FRACTION, nrFrames) );

    //Example format filename : 'dwf-b',I2,'-d',I3,'-rho',F4.2,'-r',F5.3,'-f',F5.3,'-WA.',I2,'.csv';
    std::stringstream sstr;
    sstr << std::fixed;//Print trailing zeroes
    sstr << "dwftrace";
    sstr << "-b" << std::setw(2) << BLOCK_SIZE;
    sstr << "-d" << std::setw(3) << DCHOICES_D;
    sstr << "-sf" << std::setw(4) << std::setprecision(2) << SPARE_FACTOR;
    sstr << "-r" << std::setw(5) << std::setprecision(3) << HOT_REQUEST_RATIO;
    sstr << "-f" << std::setw(5) << std::setprecision(3) << HOT_FRACTION;
    sstr << "-" << traceID;
    std::string fileName = sstr.str();

    for(uint run = startrun; run < (startrun+nruns); run ++ )
    {
        RandNrGen::getInstance().reset();
        HotColdID *hcID = new Oracle_HCID(events, oracle);

        Ssd ssd(SSD_SIZE, hcID);
        ssd.initialize(events, oracle);

        const Controller &ctrl = ssd.get_controller();
        uint it = 0;
        while(ctrl.stats.get_currentPE() > maxPE)
        {
            const Event &evt = events[it];
            ssd.event_arrive(evt.get_event_type(), evt.get_logical_address(), evt.get_size(), evt.get_start_time());//Timings don't really matter for PE fairness/SSD endurance
            it = (it + 1) % numRequests;
            hcID->next_request();
        }
        ssd.write_statistics_csv(fileName, run);
    }
    return 0;
}
