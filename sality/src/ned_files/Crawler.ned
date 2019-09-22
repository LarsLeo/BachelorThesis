package sality.ned_files;

simple Crawler
{
    parameters:
        int version = default(2);
        int pollPhaseDelay = default(6);
        int filterPhaseDelay = default(3);
    gates:
	    inout outputGate[];
	    inout inputGate[]; // for the case that the crawler is also a sensor.
}