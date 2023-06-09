#!/bin/bash

output_file="numba.log"
process_name="python main_numba.py"

while true; do
    pid=$(pgrep -f "$process_name")
    
    if [ -n "$pid" ]; then
        ps -p "$pid" -o pid,%mem,%cpu,rss | grep -v PID | awk '{print $1,$2,$3,$4/1024}' | tee -a "$output_file"
    fi
    
    sleep 0.1
done
