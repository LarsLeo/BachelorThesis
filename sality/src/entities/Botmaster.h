#ifndef ENTITIES_BOTMASTER_H_
#define ENTITIES_BOTMASTER_H_

#include "Superpeer.h"

namespace sality {

class Botmaster : public Superpeer {
protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;

private:
    const char * newURLPackMessage = "NewURLPackMessage";
    int maxNewURLPackDelay = 20; // possible amount of simulation time to pass before next URL Pack is send

    void scheduleNextURLPack();
};

}

#endif
