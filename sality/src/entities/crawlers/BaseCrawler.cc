#include "BaseCrawler.h"

Define_Module(BaseCrawler);

// This class represents an almighty crawler connected to every
// bot in the botnet.
void BaseCrawler::initialize()
{
    for (int i = 0; i < gateSize("gate"); i++) {
        possibleBotmasters.push_front(i);
    }
}

void BaseCrawler::handleMessage(cMessage* msg)
{
}

void BaseCrawler::pollSuperpeers() {
    cMessage* msg = new cMessage(SalityConstants::urlPackMessage);
    std::list<int>::iterator it;

//    for (it = possibleBotmasters.begin(); it != possibleBotmasters.end(); it++) {
//        cMessage *dup = msg->dup();
//        dup->setTimestamp();
//        forwardMessage(dup, "outputGate$o", i);
//    }
}
