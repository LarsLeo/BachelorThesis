#/bin/bash

for i in $(seq 1 4)
do
	if ls ./results/BV$i* 1> /dev/null 2>&1; then
   		echo "zipping files for BV$i ..."
		tar -cvzf tarballResults/BV$i-$(date +%F_%T).tar.gz results/BV$i*
	elif ls ./results/Crawler-PS$i* 1> /dev/null 2>&1; then
		echo "zipping files for Crawler-PS$i"
		tar -cvzf tarballResults/Crawler-PS$i-$(date +%F_%T).tar.gz results/Crawler-PS$i*
	else
		echo "files do not exist"
	fi
done
