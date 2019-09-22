#include "Botmaster.h"

Define_Module(Botmaster);

void Botmaster::initialize(){
    botmasterVersion = par("botmasterVersion");
    peerSelectVersion = par("peerSelectVersion");
    distributionPercentage = par("distributionPercentage");
    urlPackDelay = par("urlPackDelay");
    urlPackOffset = par("urlPackOffset");
    crawlerEnabled = par("crawlerEnabled");

    chosenPeerPercentage = par("chosenPeerPercentage"); // Only used by peerSelectVersion == 4

    int numPeers = gateSize("outputGate");
    EV_INFO << "number peers: " << numPeers << endl;

    if (botmasterVersion == 1) {
        botmasterPeer = intrand(numPeers);
    } else {
        calculatePeerOffset(numPeers);
    }

    if (peerSelectVersion >= 3) {
        gatherPeerOffsets();
    }

    if (crawlerEnabled) logPeerlist();

    scheduleNextURLPack();
}

void Botmaster::logPeerlist() {
    // only peerSelectVersions 2 and 4 are used in the crawler simulation
    EV_INFO << "botmaster peers:";
    for (int i = 0; i <= lastKnownPeerIndex; i += peerOffset) {
        const cModule &superpeer = *gate("outputGate$o", i)->getPathEndGate()->getOwnerModule();
        EV_INFO << superpeer.getId() << ",";
    }
    EV_INFO << endl;
}

void Botmaster::gatherPeerOffsets() {
    cMessage* msg = new cMessage(SalityConstants::mmProbe);
    int numberPeersProbed = (peerSelectVersion == 3) ? gateSize("outputGate") - 1 : lastKnownPeerIndex;

    for (int i = 0; i <= numberPeersProbed; i += 1) {
        sendMessageDup(msg, i);
    }

    delete msg;
}

void Botmaster::calculatePeerOffset(int numPeers) {
    int numPeersKnown = numPeers * (float(distributionPercentage) / 100);
    numPeersKnown = numPeersKnown > 0 ? numPeersKnown : 1;

    if (peerSelectVersion == 1) {
        lastKnownPeerIndex = gateSize("outputGate$o") - 1;
        peerOffset = numPeers / numPeersKnown;
    } else {
        lastKnownPeerIndex = numPeersKnown - 1;
        peerOffset = 1;
    }
}

void Botmaster::scheduleNextURLPack() {
    int nextURLPackOffset = intrand(urlPackOffset);
    cMessage *newURLPack = new cMessage(SalityConstants::newURLPackMessage);
    scheduleAt(simTime() + urlPackDelay - nextURLPackOffset, newURLPack);
}

void Botmaster::pushToBotmasterPeer() {
    Url_pack *urlMessage = new Url_pack(SalityConstants::urlPackMessage);
    urlMessage->setSequenceNumber(sequenceNumber);
    // sends directly without delay so that the peer mimics the botmaster.
    // Still the channel delay exists, which is no influence however.
    send(urlMessage, "outputGate$o", botmasterPeer);
}

// Active Botmaster 1
void Botmaster::pushWithProtocol() {
    Url_pack *msg = new Url_pack(SalityConstants::urlPackProbeMessage);
    msg->setSequenceNumber(sequenceNumber);
    broadcastMessage(msg);
}

// Active Botmaster 2
void Botmaster::pushDirectly() {
    Url_pack *urlMessage = new Url_pack(SalityConstants::urlPackMessage);
    urlMessage->setSequenceNumber(sequenceNumber);
    broadcastMessage(urlMessage);
}

void Botmaster::handleMessage(cMessage* msg) {
    if (strcmp(SalityConstants::newURLPackMessage, msg->getName()) == 0) {
        sequenceNumber++;
        EV_INFO << "botmaster: seq=" << sequenceNumber << " t=" << simTime() << "\n";
        switch (botmasterVersion) {
            case 1: pushToBotmasterPeer(); break;
            case 2: pushWithProtocol(); break;
            case 3: pushDirectly();
        }
        scheduleNextURLPack();
    } else if (strcmp(SalityConstants::urlPackProbeMessage, msg->getName()) == 0) {
        handlePeerProbe(msg);
    } else if (strcmp(SalityConstants::mmReply, msg->getName()) == 0) {
        addPeerOffset(check_and_cast<Start_Offset*>(msg));
    }

    delete msg;
}

void Botmaster::addPeerOffset(Start_Offset* msg) {
    int offset = msg->getOffset();
    int peerId = msg->getArrivalGate()->getIndex();

    mmOffsetTable.insert(pair<int, int>(peerId, offset));
}

void Botmaster::forwardMessage(cMessage* msg, int gate) {
    float delay = MessageDelayGenerator::getGeometricMessageDelay();
    sendDelayed(msg, delay, "outputGate$o", gate);
}

void Botmaster::broadcastMessage(cMessage* msg) {
    if (peerSelectVersion < 3) {
        for (int i = 0; i <= lastKnownPeerIndex; i += peerOffset) {
            sendMessageDup(msg, i);
        }
    } else {
        calculateNextSuperpeers();
        int numberKnownPeers = (peerSelectVersion == 3) ? lastKnownPeerIndex :
                ((int) lastKnownPeerIndex * (chosenPeerPercentage / 100.0));
        for (int i = 0; i <= numberKnownPeers - 1; i++) {
            sendMessageDup(msg, vec[i].first);
        }
    }

    delete msg;
}

void Botmaster::sendMessageDup(cMessage* msg, int index) {
    cMessage* dup = msg->dup();
    dup->setTimestamp();
    forwardMessage(dup, index);
}

// calculates the peers, that have the closest MM cycle and pushes them to nextPeers.
void Botmaster::calculateNextSuperpeers() {
    nextMMCycles.clear();
    vec.clear();

    for (it = mmOffsetTable.begin(); it != mmOffsetTable.end() ;) {
        int peerId = it->first;
        int offset = it->second;

        int nextCycleDelay = fmod(simTime().dbl(), offset);
        nextMMCycles.insert(pair<int, int>(peerId, nextCycleDelay));

        it++;
    }

    // copy key-value pairs from the map to the vector
    std::copy(nextMMCycles.begin(), nextMMCycles.end(), back_inserter<vector<pair<int, int>>>(vec));

    // sort the vector by second value in increasing order
    std::sort(vec.begin(), vec.end(), [](const pair<int, int>& l, const pair<int, int>& r) {
        if (l.second != r.second)
            return l.second < r.second;

        return l.first < r.first;
    });
}

// Used for botmasterVersion 2, answering probe messages
void Botmaster::handlePeerProbe(cMessage *msg) {
    Url_pack *urlMessage = new Url_pack(SalityConstants::urlPackMessage);
    urlMessage->setSequenceNumber(sequenceNumber);
    forwardMessage(urlMessage, msg->getArrivalGate()->getIndex());
}
