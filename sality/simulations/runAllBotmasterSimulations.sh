#!/bin/bash

# do 1..3 once botmaster version 1 works
for i in {2..3}
do
	source runSimulation.sh BV$i
done
