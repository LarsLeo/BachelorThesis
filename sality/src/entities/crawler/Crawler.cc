#include "Crawler.h"

Define_Module(Crawler);

void Crawler::initialize()
{
    version = par("version");

    switch(version) {
        case 1: initV1Crawler(); break;
    }

    EV_INFO << "number peers: " << gateSize("gate") << endl;
    for (int i = 0; i < gateSize("gate"); i++) {
        possibleBotmasters.push_front(i);
    }

    pollSuperpeers();
    scheduleCrawlerCycle();
}

void Crawler::scheduleCrawlerCycle() {
    cMessage *nextCycle = new cMessage(SalityConstants::newCrawlerCycle);
    scheduleAt(simTime() + cycleDelay, nextCycle);
}

// V1 works by pulling the sequence numbers in cycles:
// 1. Poll the sequence numbers of all bots in possibleBotmasters.
// 2. Wait for each response and save it in a table.
// 3. After 5 seconds, get seqMax from the table.
// 4. Iterate through the table and discard all bots that have a seqNumber < max.
// 5. Restart at 1 until convergence.
void Crawler::initV1Crawler() {
    for (int i = 0; i < gateSize("gate"); i++) {
        botmasterTable.insert(std::pair<int, int>(i, 1)); ;
    }
}

void Crawler::forwardMessage(cMessage *msg,  int gate) {
    float delay = MessageDelayGenerator::getGeometricMessageDelay();
    sendDelayed(msg, delay, "gate$o", gate);
}

void Crawler::handleMessage(cMessage *msg) {
    if (strcmp(SalityConstants::urlPackMessage, msg->getName()) == 0) {
        addResponseEntry(check_and_cast<Url_pack *>(msg));
    } else if (strcmp(SalityConstants::newCrawlerCycle, msg->getName()) == 0) {
        updatePossibleBotmasters();
        pollSuperpeers();
    }
    delete msg;
}

void Crawler::addResponseEntry(Url_pack *msg) {
    int currentSeq = botmasterTable[msg->getArrivalGate()->getIndex()];

    if (version == 1 && currentSeq < msg->getSequenceNumber()) {
        botmasterTable[msg->getArrivalGate()->getIndex()] = msg->getSequenceNumber();
    }
}

void Crawler::pollSuperpeers() {
    Url_pack * urlMessage = new Url_pack(SalityConstants::urlPackProbeMessage);
    urlMessage->setSequenceNumber(0);

    for (it = botmasterTable.begin(); it != botmasterTable.end(); it++) {
        cMessage * dup = urlMessage->dup();
        forwardMessage(dup, it->first);
    }
}

void Crawler::updatePossibleBotmasters() {
    switch (version) {
        case 1: v1TableUpdate(); break;
    }
    scheduleCrawlerCycle();
}

void Crawler::v1TableUpdate() {
    int currentMax = 1;

    // Iterate through the seqTable and find currentMax, already deleting entries.
    for (it = botmasterTable.begin(); it != botmasterTable.end() ;) {
        if (it->second >= currentMax) {
            currentMax = it->second;
            it++;
        } else {
            it = botmasterTable.erase(it);
        }
    }

    // Iterate a second time to erase entries that may have been higher at previous positions.
    for (it = botmasterTable.begin(); it != botmasterTable.end() ;) {
        if (it->second >= currentMax) {
            it++;
        } else {
            it = botmasterTable.erase(it);
        }
    }

//   if (botmasterTable.size() == 1) {
//       EV_INFO << "Crawler found botmaster: " << botmasterTable.begin()->first;
//   }
}
