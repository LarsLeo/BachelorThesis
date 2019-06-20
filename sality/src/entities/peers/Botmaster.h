#ifndef ENTITIES_BOTMASTER_H_
#define ENTITIES_BOTMASTER_H_

#include "Superpeer.h"
#include "../utility/SalityConstants.h"

class Botmaster : public Superpeer {
protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;

private:
    int maxNewURLPackDelay; // possible amount of simulation time in seconds to pass before next URL Pack is send

    void scheduleNextURLPack();
    void publishNewURLPack();
};

#endif
