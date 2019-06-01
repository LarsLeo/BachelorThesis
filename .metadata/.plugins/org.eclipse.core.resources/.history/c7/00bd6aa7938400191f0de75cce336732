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
    sequenceNumber++;
    EV << "Botmaster increased sequence number to: " << sequenceNumber << "\n";
    int nextURLPackOffset = rand() % maxNewURLPackDelay;
    cMessage *newURLPack = new cMessage(newURLPackMessage);
    scheduleAt(simTime() + maxNewURLPackDelay - nextURLPackOffset, newURLPack);
}

void Botmaster::handleMessage(cMessage *msg) {
    if (strcmp(newURLPackMessage, msg->getName()) == 0) {
        scheduleNextURLPack();
    } else {
        Superpeer::handleMessage(msg);
    }
}
