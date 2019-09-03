#ifndef ENTITIES_PEERS_BOTMASTER_H_
#define ENTITIES_PEERS_BOTMASTER_H_

#include <omnetpp.h>
#include "../../messages/url_pack_m.h"
#include "../../messages/start_offset_m.h"
#include "../utility/SalityConstants.h"
#include "../utility/MessageDelayGenerator.h"
#include <algorithm>
#include <string>

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
    int botmasterVersion; // Botmaster distribution version
    int distributionPercentage; // Percentage of known peers if active
    int peerOffset;
    int lastKnownPeerIndex;
    int botmasterPeer; // If botmaster version == 1, this peer denotes which of the superpeers plays the botmaster
    int peerSelectVersion; // 1 = random, 2 = most connections, 3 = next MM cycle, 4 = mix connections/MM
    bool crawlerEnabled;

    void calculatePeerOffset(int numPeers);
    void pushToBotmasterPeer(); // Passive
    void pushWithProtocol(); // Active 1
    void pushDirectly(); // Active 2
    void handlePeerProbe(cMessage* msg);
    void sendMessageDup(cMessage* msg, int index);

    // If peerSelectVersion == 3 || 4:
    map<int, int> mmOffsetTable; // Holds a mapping for each known superpeer to its time it joined the botnet.
    map<int, int> nextMMCycles; // Each iteration calculates the peers delays to the next MM cycle.
    vector<pair<int, int>> vec; // Holds a sorted version of nextMMCycles.
    map<int, int>::iterator it;
    void gatherPeerOffsets();
    void addPeerOffset(Start_Offset* msg);
    void calculateNextSuperpeers(); // Calculates the superpeers, that have the next MM cycles.

    // If peerSelectVersion == 4:
    int chosenPeerPercentage; // The percentage the botmaster chooses to distribute the URL pack towards.

    // If a crawler is enabled
    void logPeerlist();
};

#endif
