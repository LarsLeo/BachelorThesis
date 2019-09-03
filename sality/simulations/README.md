# Base folder for running simulations as well as the results

### runSimulation.sh:
- Arguments: simulationConfigName as defined in the omnetpp.ini file.
- Runs the singular simulation of this specific configuration.

### runAllForPS:
- Arguments: PS the peer select version of the botmaster.
- Runs the simulations configured by all configurations that include the PS.

### fileZipper.sh:
- Stores the output of simulations of ./results in tarballs that are saved in ./tarballResults.
