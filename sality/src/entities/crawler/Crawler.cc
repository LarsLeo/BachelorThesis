#include "Crawler.h"

Define_Module(Crawler);

void Crawler::initialize() {
    version = par("version");

    switch (version) {
        case 1: initV1Crawler(); break;
        case 2: initV2Crawler(); break;
    }

    pollSuperpeers();
}

void Crawler::scheduleCrawlerCycle() {
    cMessage *nextCycle = new cMessage(SalityConstants::newCrawlerCycle);
    scheduleAt(simTime() + cycleDelay, nextCycle);
}

void Crawler::forwardMessage(cMessage *msg,  int gate) {
    float delay = MessageDelayGenerator::getGeometricMessageDelay();
    sendDelayed(msg, delay, "gate$o", gate);
}

void Crawler::handleMessage(cMessage *msg) {
    if (strcmp(SalityConstants::crawlerPoll, msg->getName()) == 0) {
        addResponseEntry(check_and_cast<CrawlerPoll *>(msg));
    } else if (strcmp(SalityConstants::newCrawlerCycle, msg->getName()) == 0) {
        pollSuperpeers();
    }

    delete msg;
}

void Crawler::addResponseEntry(CrawlerPoll *msg) {
    responsesLeft--;
    int index = msg->getArrivalGate()->getIndex();
    int messageSeq = msg->getSequenceNumber();
    if (messageSeq > maxSeqNumber) {
        newPackReleased = true;
        maxSeqNumber = messageSeq;
    }

    if (version == 1) {
        if (msg->getType() == 'b')
            botmasterTable[index] = messageSeq;
        else
            offlinePeers[index] = messageSeq;
    }

    if (responsesLeft == 0) {
        responsesLeft = -1;
        if (newPackReleased) {
            updatePossibleBotmasters();
            newPackReleased = false;
        }
        scheduleCrawlerCycle();
    }
}

void Crawler::pollSuperpeers() {
    CrawlerPoll *pollMessage = new CrawlerPoll(SalityConstants::crawlerPoll);
    pollMessage->setSequenceNumber(0);

    switch (version) {
        case 1: v1Poll(pollMessage); break;
        case 2: v2Poll(pollMessage); break;
    }

    delete pollMessage;
}

void Crawler::updatePossibleBotmasters() {
    switch (version) {
        case 1: v1TableUpdate(); break;
        case 2: v2TableUpdate(); break;
    }
}

void Crawler::logPeerlist() {
    EV_INFO << "crawler peers at t=" << (int) simTime().dbl() << ":";

    switch (version) {
        case 1: v1peerlistLog(); break;
        case 2: v2peerlistLog(); break;
    }

    EV_INFO << endl;
}

// Crawler V1 section
// V1 works by pulling the sequence numbers in cycles:
// 1. Poll the sequence numbers of all bots in possibleBotmasters.
// 2. Wait for each response and save it in a table.
// 3. After 5 seconds, get seqMax from the table.
// 4. Iterate through the table and discard all bots that have a seqNumber < max.
// 5. Restart at 1 until convergence.
void Crawler::initV1Crawler() {
    for (int i = 0; i < gateSize("gate"); i++) {
        botmasterTable.insert(pair<int, int>(i, 1));
    }
}

void Crawler::v1TableUpdate() {
    bool tableChanged = false;

    // peers are moved rather than put in temp map to save RAM
    for (it = offlinePeers.begin(); it != offlinePeers.end();) {
        if (it->second == maxSeqNumber) {
            botmasterTable[it->first] = it->second;
            tableChanged = true;
        }
        it = offlinePeers.erase(it);
    }

    // move peers < global max
    for (it = botmasterTable.begin(); it != botmasterTable.end();) {
        if (it->second < maxSeqNumber) {
            offlinePeers[it->first] = it->second;
            it = botmasterTable.erase(it);
            tableChanged = true;
        } else it++;
    }

    if (tableChanged) logPeerlist();
}

void Crawler::v1Poll(CrawlerPoll *pollMessage) {
    responsesLeft = botmasterTable.size() + offlinePeers.size();

    for (it = botmasterTable.begin(); it != botmasterTable.end(); it++) {
        CrawlerPoll *dup = pollMessage->dup();
        dup->setType('b');
        forwardMessage(dup, it->first);
    }

    for (it = offlinePeers.begin(); it != offlinePeers.end(); it++) {
        CrawlerPoll *dup = pollMessage->dup();
        dup->setType('o');
        forwardMessage(dup, it->first);
    }
}

void Crawler::v1peerlistLog() {
    for (it = botmasterTable.begin(); it != botmasterTable.end(); it++) {
        const cModule &superpeer = *gate("gate$o", it->first)->getPathEndGate()->getOwnerModule();
        EV_INFO << superpeer.getId() << ",";
    }
}

// Crawler V2 section
void Crawler::initV2Crawler() {
}

void Crawler::v2TableUpdate() {

}

void Crawler::v2Poll(CrawlerPoll *pollMessage) {

}

void Crawler::v2peerlistLog() {

}
