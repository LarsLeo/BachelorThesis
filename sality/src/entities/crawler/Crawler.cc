#include "Crawler.h"

Define_Module(Crawler);

void Crawler::initialize() {
    version = par("version");

    switch (version) {
        case 1: initV1Crawler(); break;
        case 2: initV2Crawler(); break;
    }

    startPollingPhase();
    pollSuperpeers();
}

void Crawler::scheduleCrawlerCycle() {
    cMessage *nextCycle = new cMessage(SalityConstants::newCrawlerCycle);
    scheduleAt(simTime() + cycleDelay, nextCycle);
}

void Crawler::forwardMessage(cMessage * msg,  int gate) {
    float delay = MessageDelayGenerator::getGeometricMessageDelay();
    sendDelayed(msg, delay, "outputGate$o", gate);
}

void Crawler::handleMessage(cMessage * msg) {
    if (strcmp(SalityConstants::crawlerPoll, msg->getName()) == 0)
        handleResponseEntry(check_and_cast<CrawlerPoll *>(msg));
    else if (strcmp(SalityConstants::newCrawlerCycle, msg->getName()) == 0)
        pollSuperpeers();
    else if (strcmp(SalityConstants::urlPackProbeMessage, msg->getName()) == 0) // only for sensor
        evaluatePeerResponse<Url_pack *>(check_and_cast<Url_pack *>(msg));
    else if (strcmp(SalityConstants::crawlerFilterEnd, msg->getName()) == 0) {
        updatePossibleBotmasters();
        logPeerlist();
        startPollingPhase();
    }

    delete msg;
}

void Crawler::handleResponseEntry(CrawlerPoll * msg) {
    switch (version) {
        case 1: handleResponseV1(msg); break;
        case 2: handleResponseV2(msg); break;
    }
}

void Crawler::startPollingPhase() {
    cycleDelay = par("pollPhaseDelay");
    filterPhaseActive = false;
    initialTimestamp = -1;
}

void Crawler::startFilterPhase() {
    cMessage * filterEnd = new cMessage(SalityConstants::crawlerFilterEnd);
    scheduleAt(simTime() + 60, filterEnd);

    cycleDelay = par("filterPhaseDelay");
    filterPhaseActive = true;
}

void Crawler::pollSuperpeers() {
    CrawlerPoll *pollMessage = new CrawlerPoll(SalityConstants::crawlerPoll);
    pollMessage->setSequenceNumber(0);

    switch (version) {
        case 1: v1Poll(pollMessage); break;
        case 2: v2Poll(pollMessage); break;
    }

    scheduleCrawlerCycle();

    delete pollMessage;
}

void Crawler::updatePossibleBotmasters() {
    switch (version) {
        case 1: v1TableUpdate(); break;
        case 2: v2TableUpdate(); break;
    }
}

void Crawler::logPeerlist() {
    EV_INFO << "GoodCounts after pack: " << maxSeqNumber << ":\t";

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
    for (int i = 0; i < gateSize("outputGate"); i++) {
        v1Table.insert(make_pair(i, 1));
    }
}

void Crawler::handleResponseV1(CrawlerPoll * msg) {
    //    responsesLeft--;
    //    int index = msg->getArrivalGate()->getIndex();
    //    int messageSeq = msg->getSequenceNumber();
    //    if (messageSeq > maxSeqNumber) {
    //        newPackReleased = true;
    //        maxSeqNumber = messageSeq;
    //    }
    //
    //    if (version == 1) {
    //        if (msg->getType() == 'b')
    //            v1Table[index] = messageSeq;
    //        else
    //            offlinePeers[index] = messageSeq;
    //    } else if (version == 2) {
    //        int timestamp = (int) msg->getTimestamp().dbl();
    ////        EV_INFO << timestamp << endl;
    //        v2Table[index] = make_pair(messageSeq, timestamp);
    //    }
    //
    //    if (responsesLeft == 0) {
    //        if (newPackReleased) {
    //            updatePossibleBotmasters();
    //            newPackReleased = false;
    //        }
    //
    //        responsesLeft = -1;
    //        scheduleCrawlerCycle();
    //    }
}

void Crawler::v1TableUpdate() {
    bool tableChanged = false;

    // peers are moved rather than put in temp map to save RAM
    for (v1It = offlinePeers.begin(); v1It != offlinePeers.end();) {
        if (v1It->second == maxSeqNumber) {
            v1Table[v1It->first] = v1It->second;
            tableChanged = true;
        }
        v1It = offlinePeers.erase(v1It);
    }

    // move peers < global max
    for (v1It = v1Table.begin(); v1It != v1Table.end();) {
        if (v1It->second < maxSeqNumber) {
            offlinePeers[v1It->first] = v1It->second;
            v1It = v1Table.erase(v1It);
            tableChanged = true;
        } else v1It++;
    }

    if (tableChanged) logPeerlist();
}

void Crawler::v1Poll(CrawlerPoll *pollMessage) {
    for (v1It = v1Table.begin(); v1It != v1Table.end(); v1It++) {
        CrawlerPoll *dup = pollMessage->dup();
        dup->setType('b');
        forwardMessage(dup, v1It->first);
    }

    for (v1It = offlinePeers.begin(); v1It != offlinePeers.end(); v1It++) {
        CrawlerPoll *dup = pollMessage->dup();
        dup->setType('o');
        forwardMessage(dup, v1It->first);
    }
}

void Crawler::v1peerlistLog() {
    for (v1It = v1Table.begin(); v1It != v1Table.end(); v1It++) {
        const cModule & superpeer = *gate("outputGate$o", v1It->first)->getPathEndGate()->getOwnerModule();
        EV_INFO << superpeer.getId() << ",";
    }
}

// Crawler V2 section
void Crawler::initV2Crawler() {
    for (int i = 0; i < gateSize("outputGate"); i++) {
        int superpeerId = (*gate("outputGate$o", i)->getPathEndGate()->getOwnerModule()).getId();
        v2Table.insert(make_pair(superpeerId, make_tuple(i, 1, -1)));
        goodCounts.insert(make_pair(superpeerId, 0));
    }
}

void Crawler::handleResponseV2(CrawlerPoll * msg) {
    evaluatePeerResponse<CrawlerPoll *>(msg);
}

void Crawler::v2TableUpdate() {
    v2TimestampVec.clear();

    for (auto it = v2Table.begin(); it != v2Table.end(); it++) {
       int peerId = it->first;
       int goodCount = goodCounts[peerId];
       int seqNum = get<1>(it->second);
       int timestamp = get<2>(it->second);
       int delta = timestamp - initialTimestamp;

       goodCounts[peerId] = (delta <= maxReceivingDelay) ? goodCount + 1 : goodCount - 1;
//       if (seqNum == maxSeqNumber)
//           v2TimestampVec.push_back(pair<int, int>(peerId, timestamp));
    }

//    std::sort(v2TimestampVec.begin(), v2TimestampVec.end(), [](const pair<int, int> & l, const pair<int, int> & r) {
//       if (l.second != r.second)
//           return l.second < r.second;
//
//       return l.first < r.first;
//    });

//    int initialTimestamp = 0;
//    int currentTimestamp = 0;
//    for (auto it = v2TimestampVec.begin(); it != v2TimestampVec.end(); it++) {
//        int peerId = it->first;
//        currentTimestamp = it->second;
//
//        if (initialTimestamp == 0) {
//            initialTimestamp = it->second;
//        }
//
//        int delta = currentTimestamp - initialTimestamp;
//        goodCount = goodCounts[peerId];
//        goodCounts[peerId] = (delta <= maxReceivingDelay) ? goodCount + 1 : goodCount - 1;
//    }
}

void Crawler::v2Poll(CrawlerPoll * pollMessage) {
    for (auto it = v2Table.begin(); it != v2Table.end(); it++) {
        CrawlerPoll *dup = pollMessage->dup();
        dup->setType('b');
        forwardMessage(dup, get<0>(it->second));
    }
}

void Crawler::v2peerlistLog() {
    for (auto it = goodCounts.begin(); it != goodCounts.end(); it++) {
        auto superpeerId = it->first;
        auto goodCount = it->second;
        EV_INFO << superpeerId << ":" << goodCount << ",";
    }
}
