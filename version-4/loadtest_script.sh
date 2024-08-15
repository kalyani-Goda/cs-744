#!/bin/bash

SERVER_IP=$(nslookup "$1" | awk '/^Address: / { print $2; exit }')
if [ -n "$SERVER_IP" ]; then
    echo "Starting simulation with $1"
else
    echo "Unable to resolve the IP address of $1"
    SERVER_IP=$1
fi

SERVER_PORT=$2

SOURCE_FILE="./test/test.c"

loops=$(seq 2 2 10)

directory1="graph_results"

if [ ! -d "$directory1" ]; then
    mkdir -p "$directory1"
fi

directory2="simulation_results"


REQUEST_PER_CLIENT=$3
SLEEP_TIME_SECONDS=$4
TIMEOUT_SECONDS=$5

# Create and clear the files to store the results of the analysis
touch "$directory1/th.txt"
> "$directory1/th.txt"
touch "$directory1/rt.txt"
> "$directory1/rt.txt"
touch "$directory1/gp.txt"
> "$directory1/gp.txt"
touch "$directory1/er.txt"
> "$directory1/er.txt"
touch "$directory1/tr.txt"
> "$directory1/tr.txt"
touch tmp.txt
> tmp.txt

for j in $loops; do
    bashloop=$j

    starttime=$(date +%s%N)
    # Initialize variables to store overall throughput and average response time
    TotalThroughput=0
    TotalResponseTime=0
    TotalServiceTime=0
    success_sum=0
    error_rate=0
    timeout_rate=0
    success_rate=0

    # Array to store output file paths
    output_files=()

    for ((i=1; i<=bashloop; i++)); do
        directory3=${directory2}/"${j}_clients"
        if [ ! -d "$directory3" ]; then
            mkdir -p "$directory3"
        fi
        output_file="${directory3}/${i}.txt"
        output_files+=("$output_file")
        (
            echo "./bin/client $SERVER_IP $SERVER_PORT $SOURCE_FILE $REQUEST_PER_CLIENT $SLEEP_TIME_SECONDS $TIMEOUT_SECONDS > $output_file"
            ./bin/client "$SERVER_IP" "$SERVER_PORT" "$SOURCE_FILE" "$REQUEST_PER_CLIENT" "$SLEEP_TIME_SECONDS" "$TIMEOUT_SECONDS" > "$output_file"
        ) &
	sleep 0.1
    done

    for job in $(jobs -p); do
        wait "$job"
    done

    # Loop to get the tokens from output file
    for output_file in "${output_files[@]}"; do
        tokenid=$(awk '/RequestID =/ {print $2}' "$output_file")
	tokenids+=("tokenid")
    done

    # Loop to wait for all client processes to complete
    for output_file in "${output_files[@]}"; do
        # Extract throughput and response time from each client's output
        elapsedtime=$(awk '/elapsed_time:/ {print $2}' "$output_file")
        avgresponsetime=$(awk '/avg_response_time:/ {print $2}' "$output_file")
        timeout_rate=$(awk '/timeout_count:/ {print $2}' "$output_file")
        success=$(awk '/success_count:/ {print $2}' "$output_file")
        error_rate=$(awk '/error_count:/ {print $2}' "$output_file")
        throughput=$(awk '/thourghput:/ {print $2}' "$output_file")

        # Accumulate values for overall calculation
        TotalThroughput=$(echo "scale=6; $TotalThroughput + ($throughput)" | bc)
        success_sum=$((success + success_sum))
        TotalServiceTime=$(echo "scale=6; $TotalServiceTime + ($avgresponsetime * $success)" | bc)
        TotalResponseTime=$(echo "scale=6; $TotalResponseTime + ($elapsedtime * $success)" | bc)
        success_rate=$(echo "scale=6; $success_rate + ($success / $elapsedtime)" | bc)
        error_rate=$(echo "scale=6; $error_rate + ($error_rate / $j)" | bc)
        timeout_rate=$(echo "scale=6; $timeout_rate + ($timeout_rate / $j)" | bc)
    done

    # Calculate the overall average response time
    if [ $success_sum -ne 0 ]; then
    	TotalResponseTime=$(echo "scale=6; $TotalResponseTime / $success_sum" | bc)
    fi
    TotalThroughput=$(echo "scale=6; $TotalThroughput / $j" | bc)
    ServiceRate=$(echo "scale=6; $TotalServiceTime / $j" | bc)
    success_rate=$(echo "scale=6; $success_rate / $j" | bc)
    
    echo "client_count: $bashloop" >> tmp.txt
    echo "TotalResponseTime: $TotalResponseTime" >> tmp.txt
    echo "ServiceRate: $ServiceRate" >> tmp.txt
    echo "TotalThroughput: $TotalThroughput" >> tmp.txt
    echo "success_rate: $success_rate" >> tmp.txt
    echo "error_rate: $error_rate" >> tmp.txt
    echo "timeout_rate: $timeout_rate" >> tmp.txt
    echo -n "$bashloop " >> "$directory1/th.txt"
    echo "${TotalThroughput}" >> "$directory1/th.txt"
    echo "${ServiceRate}" >> "$directory1/sr.txt"
    echo -n "$bashloop " >> "$directory1/rt.txt"
    echo "${TotalResponseTime}" >> "$directory1/rt.txt"
    echo -n "$bashloop " >> "$directory1/gp.txt"
    echo "${success_rate}" >> "$directory1/gp.txt"
    echo -n "$bashloop " >> "$directory1/er.txt"
    echo "${error_rate}" >> "$directory1/er.txt"
    echo -n "$bashloop " >> "$directory1/tr.txt"
    echo "${timeout_rate}" >> "$directory1/tr.txt"
done

# Plot the results
echo "Plotting results..."
cat "$directory1/th.txt" | graph -T png --bitmap-size "1400x1400" -g 3 -L "M clients vs Thourghput" -X "Number of clients" -Y "thourghput (Req/Sec)" -r 0.25 > "./$directory1/throughput.png"
cat "$directory1/rt.txt" | graph -T png --bitmap-size "1400x1400" -g 3 -L "M clients vs avg response time" -X "Number of clients" -Y "avg response time (sec)" -r 0.25 > "./$directory1/response_time.png"
cat "$directory1/sr.txt" | graph -T png --bitmap-size "1400x1400" -g 3 -L "M clients vs avg service time" -X "Number of clients" -Y "avg service time (Sec)" -r 0.25 > "./$directory1/service_time.png"
cat "$directory1/er.txt" | graph -T png --bitmap-size "1400x1400" -g 3 -L "M clients vs Error rate" -X "Number of clients" -Y "Error rate (Errors / Req)" -r 0.25 > "./$directory1/error_rate.png"
cat "$directory1/gp.txt" | graph -T png --bitmap-size "1400x1400" -g 3 -L "M clients vs Goodput" -X "Number of clients" -Y "Goodput (Req/Seq)" -r 0.25 > "./$directory1/goodput.png"
cat "$directory1/tr.txt" | graph -T png --bitmap-size "1400x1400" -g 3 -L "M clients vs Timeout rate" -X "Number of clients" -Y "Timeout rate (Timeout/ Req)" -r 0.25 > "./$directory1/timeout_rate.png"


echo "Done!!"
