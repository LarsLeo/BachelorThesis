#ifndef ENTITIES_PEERS_BOTMASTER_H_
#define ENTITIES_PEERS_BOTMASTER_H_

#include <omnetpp.h>
#include "../../messages/url_pack_m.h"
#include "../../messages/start_offset_m.h"
#include "../utility/SalityConstants.h"
#include "../utility/MessageDelayGenerator.h"
#include <algorithm>

using namespace omnetpp;
using namespace std;

class Botmaster : public cSimpleModule {
protected:
    void initialize() override;
    void handleMessage(cMessage* msg) override;
    void scheduleNextURLPack();
    void forwardMessage(cMessage* msg, int gate);
    void broadcastMessage(cMessage* msg);

private:
    int sequenceNumber = 1;
    int urlPackDelay; // The default time between publishing 2 URL Packs in seconds
    int urlPackOffset; // Offset for the default delay
    int version; // Botmaster version
    int distributionPercentage; // Percentage of known peers if active or number botmaster peers if passive
    int peerOffset;
    int lastKnownPeerIndex;
    int botmasterPeer; // If botmaster version == 1, this peer denotes which of the superpeers plays the botmaster
    int peerSelectVersion; // 1 = random, 2 = most connections, 3 = next MM cycle

    // This section is only used for peerSelectVersion == 3.
    map<int, int> mmOffsetTable; // Holds a mapping for each known superpeer to its time it joined the botnet.
    map<int, int> nextMMCycles; // Each iteration calculates the peers delays to the next MM cycle.
    vector<pair<int, int>> vec; // Holds a sorted version of nextMMCycles.
    map<int, int>::iterator it;
    void gatherPeerOffsets();
    void addPeerOffset(Start_Offset* msg);
    void calculateNextSuperpeers(); // Calculates the superpeers, that have the next MM cycles.

    void calculatePeerOffset(int numPeers);
    void pushToBotmasterPeer(); // Passive
    void pushWithProtocol(); // Active 1
    void pushDirectly(); // Active 2
    void handlePeerProbe(cMessage* msg);
    void sendMessageDup(cMessage* msg, int index);
};

#endif
