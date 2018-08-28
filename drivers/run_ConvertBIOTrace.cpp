/* Convert the BIOTracer traces to a more convenient format
 * Robin Verschoren 2017
 */

#include "ssd.h"
#include "util.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

typedef ulong LPN;
typedef std::map<LPN, uint> LPNCount;
typedef std::map<LPN, uint>::iterator LPNCountIter;
typedef std::map<LPN, LPN>
  LPNMap; // Maps old LPNs (first) to new LPNs (second in pair)

struct LPNNum
{
    LPN lpn;
    uint num;
    LPNNum(const LPN& l, const uint numberOfAppearances)
      : lpn(l)
      , num(numberOfAppearances)
    {
    }
};

LPNMap
getNewLPNs(const LPNCount& count)
{

    // Make priority queue
    auto cmp = [](LPNNum left, LPNNum right) { return left.num < right.num; };
    std::priority_queue<LPNNum, std::vector<LPNNum>, decltype(cmp)> Q(cmp);
    for (const auto& lpncount : count) {
        Q.push(LPNNum(lpncount.first, lpncount.second));
    }

    // Extract numHot hottest LPNs
    LPNMap map;
    ulong it = 0;
    while (!Q.empty()) {
        map[Q.top().lpn] = it++;
        Q.pop();
    }
    return map;
}

int
main(int /*argc*/, char* argv[])
{
    using ssd::Event;

    const std::string traceFile = argv[1];
    const std::string traceID = traceFile.substr(0, 4);
    const std::string outputTraceFile = argv[2];
    const unsigned long numReq = std::stod(argv[3]);

    ssd::EVENT_READER_MODE mode;
    switch (std::stoi(argv[4])) {
        case 0:
            mode = ssd::EVTRDR_SIMPLE;
            break;
        case 1:
            mode = ssd::EVTRDR_BIOTRACER;
            break;
        default:
            mode = ssd::EVTRDR_SIMPLE;
            break;
    }
    ssd::EventReader evtRdr(traceFile, numReq, mode);
    std::vector<ssd::Event> requests = evtRdr.read_events_from_trace(traceFile);

    // Every window/frame has indexes
    // Loop through requests
    std::ofstream outStream(outputTraceFile);

    // Initialise map
    LPNCount lpnInUse;
    for (const Event& evt : requests) {
        const LPN lpn = evt.get_logical_address();
        const LPNCountIter lpnIt = lpnInUse.find(lpn);
        if (lpnIt != lpnInUse.end()) {
            lpnIt->second = lpnIt->second + 1;
        } else {
            lpnInUse.insert(std::pair<LPN, int>(lpn, 1));
        }
    }

    // Determine hot LPNs in frame
    LPNMap map = getNewLPNs(lpnInUse);
    // Iterate over frame
    for (Event& evt : requests) {
        const LPN newLPN = map[evt.get_logical_address()];
        evt.set_logical_address(newLPN);
        outStream << evtRdr.write_event(evt) << std::endl;
    }

    return 0;
}
