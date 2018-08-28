/* "Oracle" file computation
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
typedef std::map<LPN, int> LPNCount;
typedef std::map<LPN, int>::iterator LPNCountIter;
typedef std::set<LPN> HotLPNs;

struct LPNNum
{
    LPN lpn;
    unsigned int num;
    LPNNum(const LPN& l, const unsigned int numberOfAppearances)
      : lpn(l)
      , num(numberOfAppearances)
    {
    }
};

HotLPNs
getHotLPNs(const LPNCount& count, const unsigned long frameSize,
           const double fHot)
{
    const unsigned long numHot =
      static_cast<unsigned long>(std::ceil(fHot * frameSize));

    // Make priority queue
    auto cmp = [](LPNNum left, LPNNum right) { return left.num < right.num; };
    std::priority_queue<LPNNum, std::vector<LPNNum>, decltype(cmp)> Q(cmp);
    for (const auto& lpncount : count) {
        Q.push(LPNNum(lpncount.first, lpncount.second));
    }

    // Extract numHot hottest LPNs
    HotLPNs hotLPNs;
    for (uint it = 0; it < numHot and !Q.empty(); it++) {
        hotLPNs.insert(Q.top().lpn);
        Q.pop();
    }
    return hotLPNs;
}

int
main(int /*argc*/, char* argv[])
{
    const double hotFraction = std::stod(argv[1]);
    const unsigned int nrFrames = std::stoi(argv[2]);
    const unsigned long numReq = std::stoul(argv[3]);
    const unsigned long frameLength = static_cast<unsigned long>(
      (numReq % nrFrames == 0) ? std::floor((double)numReq / nrFrames)
                               : std::ceil((double)numReq / nrFrames));

#ifdef VERBOSE
    std::cout << "NUM REQUESTS: " << numreq << std::endl;
    std::cout << "NUM FRAMES: " << nrFrames << std::endl;
    std::cout << "FRAME LENGTH: " << frameLength << std::endl;
    std::cout << "MAX WINDOW RANGE: " << nrFrames * frameLength << std::endl;
#endif
    const std::string traceFile = argv[4];
    const std::string traceID = traceFile.substr(0, 4);
    const std::string oracleFile = traceID + "-" + print_fract(hotFraction) +
                                   "-" + num2str(nrFrames) + "-oracle.csv";

    ssd::EVENT_READER_MODE mode;
    switch (std::stoi(argv[5])) {
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

    // Every window/frame has indexes frameidxs = ((frameit-1)*framelength+1) :
    // min(frameit*framelength, numreq); (1-based)
    // Loop through requests
    std::ofstream oracle(oracleFile);
    for (unsigned long it = 1; it <= nrFrames; it++) {
        const unsigned long startReq = ((it - 1) * frameLength);
        const unsigned long stopReq = std::min(it * frameLength, numReq);
        LPNCount lpnInUse;
        lpnInUse.clear();
        // Initialise map
        for (unsigned long reqIt = startReq; reqIt < stopReq; reqIt++) {
            const LPN lpn = requests[reqIt].get_logical_address();
            const LPNCountIter lpnIt = lpnInUse.find(lpn);
            if (lpnIt != lpnInUse.end()) {
                lpnIt->second = lpnIt->second + 1;
            } else {
                lpnInUse.insert(std::pair<LPN, int>(lpn, 1));
            }
        }

        // Determine hot LPNs in frame
        const HotLPNs hotLPNs = getHotLPNs(lpnInUse, frameLength, hotFraction);
        // Iterate over frame
        for (unsigned long reqIt = startReq; reqIt < stopReq; reqIt++) {
            const LPN lpn = requests[reqIt].get_logical_address();
            const bool lpnIsHot = hotLPNs.find(lpn) != hotLPNs.end();
            const std::string hotStr = (lpnIsHot) ? "1" : "0";
            oracle << hotStr << std::endl;
        }
#ifdef VERBOSE
        if (it == 1 or it == nrFrames)
            std::cout << "[ " << startReq << " : " << stopReq << " ["
                      << std::endl;
#endif
    }

    return 0;
}
