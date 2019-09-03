#/bin/bash

for i in $(seq 1 3)
do
	if ls ./results/simulation/BV$i* 1> /dev/null 2>&1; then
		echo "zipping files for BV$i ..."
    		tar -cvzf tarballResults/BV$i-$(date +%F_%T)-analysis.tar.gz results/simulation/BV$i*
	else
    		echo "files for BV$i do not exist"
	fi
done

