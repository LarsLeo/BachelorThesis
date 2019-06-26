#ifndef ENTITIES_BOTMASTER_H_
#define ENTITIES_BOTMASTER_H_

#include "Superpeer.h"
#include "../utility/SalityConstants.h"

class Botmaster : public Superpeer {
protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;

private:
    int urlPackDelay; // The default time between publishing 2 URL Packs in seconds.
    int urlPackOffset; // Offset for the default delay.

    void scheduleNextURLPack();
    void publishNewURLPack();
};

#endif
