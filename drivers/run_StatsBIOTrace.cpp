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

using ssd::uint;
using ssd::ulong;

typedef ulong LPN;
typedef std::map<LPN, uint> LPNCount;
typedef std::map<LPN, uint>::iterator LPNCountIter;
typedef std::map<LPN, LPN>
  LPNMap; // Maps old LPNs (first) to new LPNs (second in pair)
typedef std::vector<ssd::IOEvent> IOEvents;

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
    // std:: cout << "MAXLPN " << it - 1 << std::endl;
    return map;
}

ulong
getNumWrites(const IOEvents& events)
{
    ulong numWrites = 0UL;
    for (const ssd::IOEvent& evt : events) {
        if (evt.type == ssd::WRITE) {
            numWrites++;
        }
    }
    return numWrites;
}

double
getFracReadOnly(const IOEvents& events, const ulong numLPN)
{
    std::vector<bool> readOnly(numLPN, true);
    ulong numReadOnly = numLPN; // 0 -> maxLPN
    for (const ssd::IOEvent& evt : events) {
        if (evt.type == ssd::WRITE && readOnly[evt.lpn]) {
            readOnly[evt.lpn] = false;
            numReadOnly--;
        }
    }
    return (double)numReadOnly / (double)(numLPN);
}

int
main(int /*argc*/, char* argv[])
{
    using ssd::Event;
    using ssd::IOEvent;

    const std::string traceFile =
      argv[1]; // Preprocessed BIOtracer CSV, assume lpns from 1 to maxLPN
    const std::string traceID = traceFile.substr(0, 4);
    const unsigned long numReq = std::stod(argv[2]);

    ssd::EVENT_READER_MODE mode;
    // switch(std::stoi(argv[3]))
    //{
    //    case 0:
    //    mode = ssd::EVTRDR_SIMPLE;
    //    break;
    //    case 1:
    mode = ssd::EVTRDR_BIOTRACER;
    //    break;
    // default:
    //    mode = ssd::EVTRDR_SIMPLE;
    //    break;
    //}
    ssd::EventReader evtRdr(traceFile, numReq, mode);
    std::vector<IOEvent> events = evtRdr.read_IO_events_from_trace(traceFile);

    // Initialise map
    LPNCount lpnInUse;
    for (const IOEvent& evt : events) {
        const LPN lpn = evt.lpn;
        const LPNCountIter lpnIt = lpnInUse.find(lpn);
        if (lpnIt != lpnInUse.end()) {
            lpnIt->second = lpnIt->second + 1;
        } else {
            lpnInUse.insert(std::pair<LPN, int>(lpn, 1));
        }
    }

    LPNMap map = getNewLPNs(lpnInUse);
    const ulong numLPN = map.size(); // Because of assumption LPN in [0, maxLPN]
    const LPN maxLPN = numLPN - 1;
    const ulong numWrites = getNumWrites(events);
    const double fracReadOnly = getFracReadOnly(events, numLPN);
    // Data set statistics
    std::cout << traceID << " & " << (double)numWrites / (double)events.size()
              << " & " << events.size() << " & " << fracReadOnly << "\\"
              << std::endl;

    // Data locality of writes
    std::vector<double> hotness = { 0.1, 0.05, 0.01, 0.005, 0.001 };
    std::cout << "DATA LOC WRITES " << std::endl << "f:";
    for (const double f : hotness) {
        std::cout << f << "\t";
    }
    std::cout << std::endl << traceID;
    for (const double f : hotness) {
        const ulong maxHotLPN = std::floor(f * maxLPN);
        ulong numHot = 0UL;
        for (IOEvent& evt : events) {
            const LPN newLPN = map[evt.lpn];
            if (newLPN <= maxHotLPN && evt.type == ssd::WRITE) {
                numHot++;
            }
        }
        std::cout << " & " << (double)numHot / (double)numWrites;
    }
    std::cout << std::endl;

    return 0;
}
