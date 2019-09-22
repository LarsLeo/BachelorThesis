package sality.ned_files;

simple Botmaster
{
    parameters:
        int botmasterVersion = default(3);
        int peerSelectVersion = default(1);
        int distributionPercentage = default(1);
        int chosenPeerPercentage = default(50);
        int urlPackDelay = default(2592000);
        int urlPackOffset = default(1296000);
        bool crawlerEnabled = default(false);
    gates:
        inout outputGate[];
}