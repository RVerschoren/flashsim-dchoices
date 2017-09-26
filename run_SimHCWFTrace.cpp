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
    BLOCK_SIZE = std::stoi(argv[1]);
    DCHOICES_D = std::stoi(argv[2]);
    SPARE_FACTOR = std::stod(argv[3]);
    HOT_FRACTION = std::stod(argv[4]);//f
    const uint startrun = std::stoi(argv[5]); //startrun
    const uint nruns = std::stoi(argv[6]); //Number of runs
    const ulong maxPE = std::stoul(argv[7]); // Maximum number of PE cycles
    const ulong numRequests = std::stoul(argv[8]);
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
    BLOCK_ERASES = 10000000;

    // HCWF
    FTL_IMPLEMENTATION = 7;
    // DChoices
    GC_ALGORITHM = 3;


    std::vector<Event> events = read_event_from_trace(traceFile, readEvtFunc);
    //Example oracle filename
    std::vector<bool> oracle = read_oracle( create_oracle_filename(traceID, HOT_FRACTION, nrFrames) );

    //Example format filename : 'dwf-b',I2,'-d',I3,'-rho',F4.2,'-r',F5.3,'-f',F5.3,'-WA.',I2,'.csv';
    std::stringstream sstr;
    sstr << std::fixed;//Print trailing zeroes
    sstr << "hcwf";
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