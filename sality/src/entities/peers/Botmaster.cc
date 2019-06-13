#include "Botmaster.h"

Define_Module(Botmaster);

// This is the botmaster as a superpeer, it will periodically increase its own
// sequence number. In this configuration it will let bots pull and not push
// the new URL Pack himself.
void Botmaster::initialize() {
    Superpeer::initialize();
    scheduleNextURLPack();
}

void Botmaster::scheduleNextURLPack() {
    int nextURLPackOffset = rand() % (maxNewURLPackDelay / 2);
    cMessage *newURLPack = new cMessage(SalityConstants::newURLPackMessage);
    scheduleAt(simTime() + maxNewURLPackDelay - nextURLPackOffset, newURLPack);
}

void Botmaster::publishNewURLPack() {
    sequenceNumber++;
    EV_INFO << "botmaster: seq=" << sequenceNumber << " t=" << simTime() << "\n";
}

void Botmaster::handleMessage(cMessage *msg) {
    if (strcmp(SalityConstants::newURLPackMessage, msg->getName()) == 0) {
        publishNewURLPack();
        scheduleNextURLPack();
        delete msg;
    } else {
        Superpeer::handleMessage(msg);
    }
}
