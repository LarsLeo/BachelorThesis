#ifndef __BACHELOR_THESIS_SALITY_SUPERPEER_H
#define __BACHELOR_THESIS_SALITY_SUPERPEER_H

#include <omnetpp.h>
#include "../messages/url_pack_m.h"

using namespace omnetpp;

class Superpeer : public cSimpleModule {
protected:
    int sequenceNumber = 0;
    const char * urlPackMessage = "URLPackMessage";
    const char * nlProbeMessage = "NLProbeMessage";
    const char * nlRequestMessage = "NLRequestMessage";
    const char * nlResponseMessage = "NLResponseMessage";
    int membershipManagementDelay = 40;
    // int neighbourListSize = 1000;
    // int lowNeighbourThreshold = 980;
    // int goodCountThreshold = -30;
    // char ID[];
    // NeighbourEntry neighbourList[neighbourListSize];

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void forwardMessage(cMessage *msg, const char* gateName, int gate);
    virtual void broadcastMessage(cMessage *msg);
    void probeNeighbours();
    void handleURLPackMessage(Url_pack *msg);
    void handleNLRequestMessage();
};

#endif
