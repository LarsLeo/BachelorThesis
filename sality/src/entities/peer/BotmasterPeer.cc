#include "../peer/BotmasterPeer.h"

Define_Module(BotmasterPeer);

// This is the botmaster as a superpeer, it will periodically increase its own
// sequence number. In this configuration it will let bots pull and not push
// the new URL Pack himself. This class will only come into effect, if botmaster
// version 1 (passive approach) is selected.
void BotmasterPeer::initialize() {
    urlPackDelay = par("urlPackDelay");
    urlPackOffset = par("urlPackOffset");
    //Superpeer::initialize();
    scheduleNextURLPack();
}

void BotmasterPeer::scheduleNextURLPack() {
    int nextURLPackOffset = intrand(urlPackOffset);
    cMessage *newURLPack = new cMessage(SalityConstants::newURLPackMessage);
    scheduleAt(simTime() + urlPackDelay - nextURLPackOffset, newURLPack);
}

void BotmasterPeer::publishNewURLPack() {
    sequenceNumber++;
    EV_INFO << "botmaster: seq=" << sequenceNumber << " t=" << simTime() << "\n";
}

void BotmasterPeer::handleMessage(cMessage *msg) {
    if (strcmp(SalityConstants::newURLPackMessage, msg->getName()) == 0) {
        publishNewURLPack();
        scheduleNextURLPack();
        delete msg;
    } else {
        Superpeer::handleMessage(msg);
    }
}
