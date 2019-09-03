package sality.ned_files;

simple Superpeer
{
    parameters:
        bool crawlerEnabled = default(false);
    gates:
	    inout inputGate[];
	    inout outputGate[];
}
