# Bachelor's Thesis of Lars Grätz

## Description:

The goal of this theis is to evaluate URL Pack distribution methods in the P2P botnet Sality and create crawlers that potentially are able to traverse the network towards the botmaster. 
To do this, a simulation environment has been created in C++ (OMNeT++). 
Different crawlers can be tested by following the description below.
Additionaly propagation and other statistics can be evaluated by using the according scripts. 
Snapshots of the existing Sality botnet in form of a graphml file can be parsed and imported into the simulation environment.

### Process:

1. Parse graphml file via the sality/scripts/graphml-to-ned.py file, selecting a crawler version and a number of botmaster nodes.
2. Move the generated NED file into the sality/src/ned\_files directory and replace existing file if necessary.
3. Run the simulation, which creates a log file in sality/simulations/results.
4. Evaluate the log via the different scripts in sality/scripts.
