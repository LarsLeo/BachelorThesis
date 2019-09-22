#ifndef ENTITIES_CRAWLER_CRAWLER_H_
#define ENTITIES_CRAWLER_CRAWLER_H_

#include <omnetpp.h>
#include <algorithm>
#include "../../messages/url_pack_m.h"
#include "../../messages/crawlerPoll_m.h"
#include "../utility/SalityConstants.h"
#include "../utility/MessageDelayGenerator.h"

using namespace omnetpp;
using namespace std;

class Crawler : public cSimpleModule {
protected:
    int version; // 1 = set, 2 = timestamp, 3 = steffen
    int cycleDelay; // Polls all Superpeers every x seconds.
    int maxSeqNumber = 0;
    bool filterPhaseActive = false;
    bool newPackReleased = false;

    void initialize();
    void handleMessage(cMessage * msg);
    void handleResponseEntry(CrawlerPoll * msg);
    void handleSensorData(Url_pack * msg);
    void forwardMessage(cMessage * msg, int gate);
    void pollSuperpeers();
    void scheduleCrawlerCycle();
    void updatePossibleBotmasters();
    void logPeerlist();
    void startFilterPhase();
    void startPollingPhase();

    // template functions
    template<typename Message>
    void evaluatePeerResponse(Message msg);

    // Crawler V1 section
    map<int, int> v1Table; // Saves the current known seqNumber for each bot.
    map<int, int>::iterator v1It;
    map<int, int> offlinePeers; // Saves peers that responded with a lower seq num from the botmasterTable

    void initV1Crawler();
    void v1Poll(CrawlerPoll * pollMessage);
    void handleResponseV1(CrawlerPoll * msg);
    void v1TableUpdate();
    void v1peerlistLog();

    // Crawler V2 section
    map<int, int> goodCounts; // superpeerId : goodCount.
    map<int, tuple<int, int, int>> v2Table; // peerId : (outputGateId, seqNum, timestamp).
//    vector<pair<int, int>> v2TimestampVec; // peerId : timestamp.
    int initialTimestamp = -1; // The timestamp at which the first new pack was received.
    int maxReceivingDelay = 9; // 2 * 3 (message delay) + 3 (cycle delay)

    void initV2Crawler();
    void v2Poll(CrawlerPoll * pollMessage);
    void handleResponseV2(CrawlerPoll * msg);
    void v2TableUpdate();
    void v2peerlistLog();
};

template<typename Message>
void Crawler::evaluatePeerResponse(Message msg){
    int messageSeqNum = msg->getSequenceNumber();
    int superpeerId = msg->getSenderModuleId();
    int timestamp = (int) msg->getTimestamp().dbl();
    int outputGateId = get<0>(v2Table[superpeerId]);

    if (!filterPhaseActive && messageSeqNum > maxSeqNumber) {
        maxSeqNumber = messageSeqNum;
        v2Table[superpeerId] = make_tuple(outputGateId, messageSeqNum, timestamp);
        initialTimestamp = timestamp;
        startFilterPhase();
    } else if (filterPhaseActive) {
        int currentSeqNum = get<1>(v2Table[superpeerId]);
        if (messageSeqNum > currentSeqNum) {
            v2Table[superpeerId] = make_tuple(outputGateId, messageSeqNum, timestamp);
        }
    }
}

#endif
