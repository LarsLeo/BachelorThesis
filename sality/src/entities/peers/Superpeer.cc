#include "Superpeer.h"

Define_Module(Superpeer);

void Superpeer::initialize()
{
    // The simulation time t is given in minutes
    // Each bot is randomly assigned a start time, at which it joined the network
    int startTimeOffset = rand() % membershipManagementDelay;
    cMessage *neighbourProbeInit = new cMessage(SalityConstants::nlProbeMessage);
    scheduleAt(membershipManagementDelay - startTimeOffset, neighbourProbeInit);
}

void Superpeer::handleMessage(cMessage *msg)
{
    if (strcmp(SalityConstants::urlPackMessage, msg->getName()) == 0) {
        handleURLPackMessage(check_and_cast<Url_pack *>(msg));
    } else if (strcmp(SalityConstants::nlRequestMessage, msg->getName()) == 0){
        handleNLRequestMessage();
    } else if (strcmp(SalityConstants::nlProbeMessage, msg->getName()) == 0) {
        probeNeighbours();
    } else if (strcmp(SalityConstants::urlPackProbeMessage, msg->getName()) == 0) {
        handleURLPackProbeMessage(check_and_cast<Url_pack *>(msg));
    }
    delete msg;
}

void Superpeer::handleURLPackProbeMessage(Url_pack *msg) {
    char* outputGate;
    asprintf(&outputGate, "%s$o", msg->getArrivalGate()->getBaseName());

    if (msg->getSequenceNumber() > sequenceNumber) {
        Url_pack *urlProbeMessage = new Url_pack(SalityConstants::urlPackProbeMessage);
        urlProbeMessage->setSequenceNumber(sequenceNumber);
        forwardMessage(urlProbeMessage, outputGate, msg->getArrivalGate()->getIndex());
    } else if (msg->getSequenceNumber() < sequenceNumber) {
        Url_pack *urlMessage = new Url_pack(SalityConstants::urlPackMessage);
        urlMessage->setSequenceNumber(sequenceNumber);
        forwardMessage(urlMessage, outputGate, msg->getArrivalGate()->getIndex());
    }

    free(outputGate);
}

void Superpeer::handleURLPackMessage(Url_pack *msg) {
    if (msg->getSequenceNumber() > sequenceNumber) {
        sequenceNumber = msg->getSequenceNumber();
        EV_INFO << "peer: id=" << getIndex() << " seq=" << sequenceNumber << " t=" << simTime() << endl;
    } else if (msg->getSequenceNumber() < sequenceNumber) {
        Url_pack *urlMessage = new Url_pack(SalityConstants::urlPackMessage);
        urlMessage->setSequenceNumber(sequenceNumber);
        char* outputGate;
        asprintf(&outputGate, "%s$o", msg->getArrivalGate()->getBaseName());
        forwardMessage(urlMessage, outputGate, msg->getArrivalGate()->getIndex());
        free(outputGate);
    }
}

void Superpeer::handleNLRequestMessage() {
    int outputGates = gateSize("outputGate");
    //TODO: get one random NL list entry and send it back
}

void Superpeer::forwardMessage(cMessage *msg, const char* gateName, int gate) {
    float delay = MessageDelayGenerator::getGeometricMessageDelay();
    sendDelayed(msg, delay, gateName, gate);
}

void Superpeer::broadcastMessage(cMessage *msg) {
    int outputGates = gateSize("outputGate");

    for (int i = 0; i < outputGates; i++) {
        cMessage *dup = msg->dup();
        dup->setTimestamp();
        forwardMessage(dup, "outputGate$o", i);
    }
}

// Membership Management Cycle that is called every 40 minutes.
// TODO: GoodCount, LastOnline, size currNeighbourList check and fetch neighbours
void Superpeer::probeNeighbours() {
    // Each MM Cycle a bot will send its own sequence number to its neighbours.
    Url_pack *msg = new Url_pack(SalityConstants::urlPackProbeMessage);
    msg->setSequenceNumber(sequenceNumber);
    broadcastMessage(msg);

    // After 40 minutes start the next Management Cycle
    cMessage *neighbourProbeInit = new cMessage(SalityConstants::nlProbeMessage);
    scheduleAt(simTime() + membershipManagementDelay, neighbourProbeInit);
}
