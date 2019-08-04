#include "Botmaster.h"

Define_Module(Botmaster);

void Botmaster::initialize()
{
    version = par("version");
    distributionPercentage = par("distributionPercentage");
    urlPackDelay = par("urlPackDelay");
    urlPackOffset = par("urlPackOffset");

    int numPeers = gateSize("gate");
    EV_INFO << "number peers: " << numPeers << endl;

    if (version == 1) {
        botmasterPeer = intrand(numPeers);
    } else {
        calculatePeerOffset(numPeers);
    }
    scheduleNextURLPack();
}

void Botmaster::calculatePeerOffset(int numPeers) {
    int numPeersKnown = numPeers * (float(distributionPercentage) / 100);
    int knownPeers = numPeersKnown > 0 ? numPeersKnown : 1;
    peerOffset = numPeers / knownPeers;
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
    send(urlMessage, "gate$o", botmasterPeer);
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

void Botmaster::handleMessage(cMessage *msg) {
    if (strcmp(SalityConstants::newURLPackMessage, msg->getName()) == 0) {
        sequenceNumber++;
        EV_INFO << "botmaster: seq=" << sequenceNumber << " t=" << simTime() << "\n";
        switch (version) {
            case 1: pushToBotmasterPeer(); break;
            case 2: pushWithProtocol(); break;
            case 3: pushDirectly(); break;
        }
        scheduleNextURLPack();
    } else if (strcmp(SalityConstants::urlPackProbeMessage, msg->getName()) == 0) {
        handlePeerProbe(msg);
    }
    delete msg;
}

void Botmaster::forwardMessage(cMessage *msg, int gate) {
    float delay = MessageDelayGenerator::getGeometricMessageDelay();
    sendDelayed(msg, delay, "gate$o", gate);
}

void Botmaster::broadcastMessage(cMessage *msg) {
    for (int i = 0; i < gateSize("gate$o"); i += peerOffset) {
        cMessage *dup = msg->dup();
        dup->setTimestamp();
        forwardMessage(dup, i);
    }

    delete msg;
}

// Used for version 2, answering probe messages
void Botmaster::handlePeerProbe(cMessage *msg) {
    Url_pack *urlMessage = new Url_pack(SalityConstants::urlPackMessage);
    urlMessage->setSequenceNumber(sequenceNumber);
    forwardMessage(urlMessage, msg->getArrivalGate()->getIndex());
}
