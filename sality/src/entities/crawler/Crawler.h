#ifndef ENTITIES_CRAWLER_CRAWLER_H_
#define ENTITIES_CRAWLER_CRAWLER_H_

#include <omnetpp.h>
#include "../../messages/url_pack_m.h"
#include "../utility/SalityConstants.h"
#include "../utility/MessageDelayGenerator.h"

using namespace omnetpp;

class Crawler : public cSimpleModule {
protected:
    int version;
    int maxSeqNumber = 0;
    int cycleDelay = 100; // Polls all Superpeers every x seconds.
    std::list <int> possibleBotmasters; // Table that holds all peers possibly connected to the botmaster.
    std::map<int, int> botmasterTable; // V1 uses a map that always saves the current known seqNumber.
    std::map<int, int>::iterator it;

    void initialize();
    void handleMessage(cMessage *msg);
    void forwardMessage(cMessage *msg, int gate);
    void pollSuperpeers();
    void scheduleCrawlerCycle();

    void addResponseEntry(Url_pack *msg);
    void updatePossibleBotmasters();

    void initV1Crawler();
    void v1TableUpdate();
};

#endif
