#!/bin/bash

printf "\nTesting web server connection using SFF..\n"
printf "\n####################################\n"

printf "\nTesting SFF with small files (slow)\n\n"
for i in $(seq 1 4); do
    ../wclient localhost 8000 /huge.html &
done
wait

printf "\n###################################\n"

printf "\nTesting SFF with huge files (fast)\n\n"
for i in $(seq 1 10); do
    ../wclient localhost 8000 /small.html &
done
wait

printf "\nDone.\n"

