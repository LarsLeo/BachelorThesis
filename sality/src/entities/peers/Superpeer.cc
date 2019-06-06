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
    }
}

void Superpeer::handleURLPackMessage(Url_pack *msg) {
    if (msg->getSequenceNumber() > sequenceNumber) {
        sequenceNumber = msg->getSequenceNumber();
        //EV << "Node: " << getIndex() << " updated to: " << sequenceNumber << "\n";
    } else if (msg->getSequenceNumber() < sequenceNumber) {
        Url_pack *urlMessage = new Url_pack(SalityConstants::urlPackMessage);
        urlMessage->setSequenceNumber(sequenceNumber);
        char* outputGate;
        asprintf(&outputGate, "%s$o", msg->getArrivalGate()->getBaseName());
        //EV << getIndex() << " received older URL pack, sending newer to " << outputGate << "\n";
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
    sendDelayed(msg, simTime() + delay, gateName, gate);
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
    //EV << "Node " << getIndex() << " probing Neighbourlist at time: " << simTime() << "\n";

    // Each MM Cycle a bot will send its own sequence number to its neighbours.
    Url_pack *msg = new Url_pack(SalityConstants::urlPackMessage);
    msg->setSequenceNumber(sequenceNumber);
    broadcastMessage(msg);

    // After 40 minutes start the next Management Cycle
    cMessage *neighbourProbeInit = new cMessage(SalityConstants::nlProbeMessage);
    scheduleAt(simTime() + membershipManagementDelay, neighbourProbeInit);
}
