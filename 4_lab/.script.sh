#!/bin/bash

# for loop from 1 to 10000
for i in {1..10000}
do
    # run and get the exitcode of valgrind ./shell < 3.in > /dev/null
    valgrind --error-exitcode=111 ./shell < 3.in 1> /dev/null 2> /dev/null
    # if the exit code is not 0 then print the error
    if [ $? -ne 0 ]
    then
        echo "Error in 3.in"
        exit 1
    fi
    echo $i
done