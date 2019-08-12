#!/bin/bash

START=2
if [[ $1 -eq 1 ]]
then
  START=1
fi

for (( i=$START; i<=3; i++ ))
do
	source runSimulation.sh BV$i-PS$1
done
