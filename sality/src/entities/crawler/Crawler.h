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
    int version;
    int maxSeqNumber = 0;
    bool newPackReleased = false;
    int cycleDelay = 10; // Polls all Superpeers every x seconds.

    void initialize();
    void handleMessage(cMessage *msg);
    void forwardMessage(cMessage *msg, int gate);
    void pollSuperpeers();
    void scheduleCrawlerCycle();
    void addResponseEntry(CrawlerPoll *msg);
    void updatePossibleBotmasters();
    void logPeerlist();

    map<int, int> botmasterTable; // Saves the current known seqNumber for each bot.
    map<int, int> offlinePeers; // Saves peers that responded with a lower seq num from the botmasterTable
    map<int, int>::iterator it;
    int responsesLeft = -1; // -1 denotes no currently running cycle.

    // Crawler V1 section
    void initV1Crawler();
    void v1Poll(CrawlerPoll *pollMessage);
    void v1TableUpdate();
    void v1peerlistLog();

    // Crawler V2 section
    void initV2Crawler();
    void v2Poll(CrawlerPoll *pollMessage);
    void v2TableUpdate();
    void v2peerlistLog();
};

#endif
