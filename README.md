# Bachelor's Thesis of Lars Grätz

## Description:

The goal of this theis is to evaluate URL Pack distribution methods in the P2P botnet Sality and create crawlers that potentially are able to traverse the network towards the botmaster. <br/>
To do this, a simulation environment has been created in OMNeT++. <br/>
Snapshots of the existing Sality botnet in form of a graphml file can be parsed and imported into the simulation environment.<br/>
Once a snapshot has been imported, botnet simulations can be run by following the description below.<br/>
Afterwards statistics can be evaluated by using the according scripts. <br/>

### Folder structure:
./sality/simulations: holds bash scripts to run a simulation.<br/>
./sality/scripts: holds python scripts for evaluation simulation runs.<br/>
./sality/src: holds the OMNeT++ classes and other related sourcecode of the simulation.<br/>
./paper: holds the Latex files of the Thesis.<br/>


### Process:

1. In the scipts section parse a graphml file via the graphml-to-ned.py file.
2. Move the generated NED file into the sality/src/ned\_files directory and replace existing file if necessary.
3. In the simulations section run a simulation, which creates log files in sality/simulations/results.
4. In the scripts section evaluate the log via the according scripts depending on the simulation.
