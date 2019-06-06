#ifndef ENTITIES_CRAWLERS_BASECRAWLER_H_
#define ENTITIES_CRAWLERS_BASECRAWLER_H_

#include <omnetpp.h>
#include "../../messages/url_pack_m.h"
#include "../utility/SalityConstants.h"
#include "../utility/MessageDelayGenerator.h"

using namespace omnetpp;

class BaseCrawler : public cSimpleModule {
protected:
    int maxSeqNumber = 0;
    std::list <int> possibleBotmasters; // stores gate ID of Superpeers that possibly are a botmaster.

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void forwardMessage(cMessage *msg, int gate);
    virtual void pollSuperpeers();
};

#endif
