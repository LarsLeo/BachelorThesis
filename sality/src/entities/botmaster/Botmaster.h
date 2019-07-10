#ifndef ENTITIES_PEERS_BOTMASTER_H_
#define ENTITIES_PEERS_BOTMASTER_H_

#include <omnetpp.h>
#include "../../messages/url_pack_m.h"
#include "../utility/SalityConstants.h"
#include "../utility/MessageDelayGenerator.h"

using namespace omnetpp;

class Botmaster : public cSimpleModule {
protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void scheduleNextURLPack();
    void forwardMessage(cMessage *msg, int gate);
    void broadcastMessage(cMessage *msg);

private:
    int sequenceNumber = 1;
    int urlPackDelay; // The default time between publishing 2 URL Packs in seconds.
    int urlPackOffset; // Offset for the default delay.
    int version; // Botmaster version
    int distributionPercentage; // Percentage of known peers if active or number botmaster peers if passive
    int peerOffset;

    void calculatePeerOffset();
    void pushWithProtocol(); // Active 1
    void pushDirectly(); // Active 2
    void handlePeerProbe(cMessage *msg);
};

#endif
