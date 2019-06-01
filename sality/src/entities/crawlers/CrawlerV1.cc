#include "CrawlerV1.h"

Define_Module(CrawlerV1);

// V1 works by pulling the sequence numbers in cycles:
// 1. Poll the sequence numbers of all bots in possibleBotmasters.
// 2. Wait for each response and save it in a table.
// 3. Get seqMax from the table.
// 4. Iterate through the table and discard all bots that have a seqNumber < max.
// 5. Restart at 1 until convergence.
void CrawlerV1::initialize() {
    for (int i = 0; i < gateSize("gate"); i++) {
        botmasterTable.insert(std::pair<int, int>(i, 1)); ;
    }
    pollSuperpeers();
}

void CrawlerV1::handleMessage(cMessage *msg) {
    if (strcmp(SalityConstants::urlPackMessage, msg->getName()) == 0) {
        addResponseEntry(check_and_cast<Url_pack *>(msg));
    }
}

void CrawlerV1::addResponseEntry(Url_pack *msg) {
    responsesLeft--;
    std::map<int, int>::iterator it;
    botmasterTable[msg->getArrivalGate()->getIndex()] = msg->getSequenceNumber();

    if (responsesLeft == 0) {
        updatePossibleBotmasters();
    }
}

void CrawlerV1::pollSuperpeers() {
    Url_pack * urlMessage = new Url_pack(SalityConstants::urlPackMessage);
    urlMessage->setSequenceNumber(0);
    responsesLeft = botmasterTable.size();

    for (it = botmasterTable.begin(); it != botmasterTable.end(); it++) {
        cMessage * dup = urlMessage->dup();
        forwardMessage(dup, it->first);
    }
}

void CrawlerV1::updatePossibleBotmasters() {
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
        if (it->second == currentMax) {
            it++;
        } else {
            it = botmasterTable.erase(it);
        }
    }

    EV << botmasterTable.size();
}
