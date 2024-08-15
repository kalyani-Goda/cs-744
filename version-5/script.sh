#!/bin/bash

hostname=$1
portno=$2
numloop=$3
sleeptime=$4
timeout_param=$5
loops=`seq 5 4 100` #"1 2 3 0 100 200 300 400 500 600 700 800 900 1000 1200 1500 1800 2000 4000 6000 8000 10000 12000"

directory=results

if [ ! -d "$directory" ]; then
	mkdir $directory 
fi

for i in $loops; do
    bashloop=$i
    start=$(date +%s%N)
    process=()
    for x in `seq $bashloop`; do
	echo "starting loop $x ./bin/client $hostname $portno test/test.c $numloop $sleeptime $timeout_param >> $directory/out_$bashloop_$x.txt 2>> $directory/err_$bashloop_$x.txt"
        ./bin/client $hostname $portno test/test.c $numloop $sleeptime $timeout_param >> $directory/out_$bashloop_$x.txt  2>> $directory/err_$bashloop_$x.txt &
        pid=$!
        process+=($pid)
        sleep 1
       # ./bin/client $hostname $portno test/error.c $numloop $sleeptime $timeout_param >> $directory/err_$bashloop_$x.txt  2>> $directory/err_$bashloop_$x.txt &
       # pid=$!
       # process+=($pid)
       # sleep 1
       # ./bin/client $hostname $portno test/result.c $numloop $sleeptime $timeout_param >> $directory/res_$bashloop_$x.txt   2>> $directory/err_$bashloop_$x.txt &
       # pid=$!
        #process+=($pid)
        #sleep 1
    done

    # Wait for each child process to complete
    for pid in "${process[@]}"; do
	echo "waiting for pid $pid"
        wait $pid
    done

    end=$(date +%s%N)
    echo "Rate time was `expr $end - $start` nanoseconds." >> tmp.txt

    echo "the $bashloop client got completed"
    end=$(date +%s%N)
    timecount=$(cat $directory/* | grep "Total time" | cut -d' ' -f3)
    timeout=$(cat $directory/* | grep "timeout_count" | cut -d' ' -f19)
    success=$(cat $directory/* | grep "success count" | cut -d' ' -f11)
    error=$(cat $directory/* | grep "error_count" | cut -d' ' -f15)
    timecount_sum=$(grep "Total time" $directory/* | awk 'BEGIN {sum=0} {sum += $3} END {print sum}')
    timeout_sum=$(grep "timeout_count" $directory/* | awk 'BEGIN {sum=0} {sum += $19} END {print sum}')
    success_sum=$(grep "success count" $directory/* | awk 'BEGIN {sum=0} {sum += $11} END {print sum}')
    error_sum=$(grep "error_count" $directory/* | awk 'BEGIN {sum=0} {sum += $15} END {print sum}')
    echo "Bashloop $bashloop" >> tmp.txt
    echo "timecount $timecount timecount_sum $timecount_sum" >> tmp.txt
    echo "Timeout $timeout timeout_sum $timeout_sum" >> tmp.txt
    echo "successcount $success successcount_sum $success_sum" >> tmp.txt
    echo "errorcount $error errorcount_sum $error_sum" >> tmp.txt
    echo "Execution time was `expr $end - $start` nanoseconds." >> tmp.txt

    rm $directory/*
    sleep 1
done
