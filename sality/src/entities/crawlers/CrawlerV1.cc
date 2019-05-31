#include "CrawlerV1.h"

Define_Module(CrawlerV1);

// V1 repeaditly pulls the current URL Pack number of all bots.
// It then checks for all bots pulled, if their seq num is >= maxSeqNumber.
// If so, all other bots are pulled, if they have a number < maxSeqNumber, they are discarded.
void CrawlerV1::handleMessage(cMessage *msg)
{

}
