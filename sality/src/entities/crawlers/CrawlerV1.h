#ifndef ENTITIES_CRAWLERS_CRAWLERV1_H_
#define ENTITIES_CRAWLERS_CRAWLERV1_H_

#include "BaseCrawler.h"

class CrawlerV1 : public BaseCrawler {
protected:
    std::map<int, int> botmasterTable; // V1 uses a map that always saves the current known seqNumber.
    std::map<int, int>::iterator it;
    int responsesLeft;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void pollSuperpeers() override;
    void addResponseEntry(Url_pack *msg);
    void updatePossibleBotmasters();
};

#endif