#/bin/bash

for i in $(seq 1 3)
do
	tar -cvzf tarballResults/BV$i-$(date +%F_%T).tar.gz results/BV$i*
done

