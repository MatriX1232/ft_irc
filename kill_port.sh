#!/bin/bash
# filepath: /root/42Warsaw/Core_Curriculum/ft_irc/kill_port.sh

if [ -z "$1" ]; then
    echo "Usage: $0 <port_number>"
    exit 1
fi

PORT=$1

# Find the process ID (PID) using the port
PID=$(sudo lsof -t -i :$PORT)

if [ -z "$PID" ]; then
    echo "No process is using port $PORT."
    exit 0
fi

# Kill the process
echo "Killing process $PID using port $PORT..."
sudo kill -9 $PID

if [ $? -eq 0 ]; then
    echo "Port $PORT has been successfully closed."
else
    echo "Failed to close port $PORT."
fi