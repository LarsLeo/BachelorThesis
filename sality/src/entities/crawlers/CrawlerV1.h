#ifndef ENTITIES_CRAWLERS_CRAWLERV1_H_
#define ENTITIES_CRAWLERS_CRAWLERV1_H_

#include "BaseCrawler.h"

class CrawlerV1 : public BaseCrawler {
protected:
    virtual void handleMessage(cMessage *msg) override;
};

#endif
