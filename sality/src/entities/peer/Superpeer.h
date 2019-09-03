#ifndef __BACHELOR_THESIS_SALITY_SUPERPEER_H
#define __BACHELOR_THESIS_SALITY_SUPERPEER_H

#include <omnetpp.h>

#include "../../messages/url_pack_m.h"
#include "../../messages/crawlerPoll_m.h"
#include "../../messages/start_offset_m.h"
#include "../utility/SalityConstants.h"
#include "../utility/MessageDelayGenerator.h"

using namespace omnetpp;

class Superpeer : public cSimpleModule {
protected:
    int sequenceNumber = 1;
    int membershipManagementDelay = 2400; // Given in seconds, all 40 minutes it is probed, so 2400.
    int startTimeOffset;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void forwardMessage(cMessage *msg, const char *gateName, int gate);
    virtual void broadcastMessage(cMessage *msg);
    void probeNeighbours();
    void retransmitMessage(cMessage *msg, cMessage *response);
    void handleURLPackMessage(Url_pack *msg);
    void handleURLPackProbeMessage(Url_pack *msg);
    void handleNLRequestMessage();
    void handleMMProbe(cMessage *msg);
    void handleCrawlerPoll(CrawlerPoll *msg);

    // If crawler is enabled
    bool crawlerEnabled;
};

#endif
