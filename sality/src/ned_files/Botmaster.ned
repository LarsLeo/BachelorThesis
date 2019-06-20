package sality.ned_files;

simple Botmaster
{
    parameters:
        int maxNewURLPackDelay = default(7600);
    gates:
        inout inputGate[];
        inout outputGate[];
}