package sality.ned_files;

simple BotmasterPeer
{
    parameters:
        int urlPackDelay = default(2592000);
        int urlPackOffset = default(1296000);
        
    gates:
        inout inputGate[];
        inout outputGate[];
}