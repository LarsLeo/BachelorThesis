#ifndef ENTITIES_BOTMASTER_H_
#define ENTITIES_BOTMASTER_H_

#include "Superpeer.h"
#include "SalityConstants.h"

class Botmaster : public Superpeer {
protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;

private:
    int maxNewURLPackDelay = 20; // possible amount of simulation time to pass before next URL Pack is send

    void scheduleNextURLPack();
};

#endif
