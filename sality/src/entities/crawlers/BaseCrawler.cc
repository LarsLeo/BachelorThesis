#include "BaseCrawler.h"

Define_Module(BaseCrawler);

// This class represents an almighty crawler connected to every
// bot in the botnet. It is only used as a superclass for specified crawlers.
void BaseCrawler::initialize()
{
    for (int i = 0; i < gateSize("gate"); i++) {
        possibleBotmasters.push_front(i);
    }
    pollSuperpeers();
}

void BaseCrawler::handleMessage(cMessage* msg){}

void BaseCrawler::forwardMessage(cMessage *msg,  int gate) {
    send(msg, "gate$o", gate);
}

void BaseCrawler::pollSuperpeers() {
    Url_pack * urlMessage = new Url_pack(SalityConstants::urlPackMessage);
    urlMessage->setSequenceNumber(0);
    std::list<int>::iterator it;

    for (it = possibleBotmasters.begin(); it != possibleBotmasters.end(); it++) {
        cMessage * dup = urlMessage->dup();
        forwardMessage(dup, *it);
    }
}
