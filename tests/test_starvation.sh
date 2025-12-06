#!/bin/bash
#
printf "\nTesting Starvation in Web Server\n";
printf "\n###############################\n";


printf "\nSending ONE huge request...\n";
../wclient localhost 8000 /huge.html &

sleep 0.2

print "\nSending MANY small requests...\n";
for i in $(seq 1 50); do
	../wclient localhost 8000 /small.html &
	sleep 0.1
done

wait

printf "\nDone.\n"


