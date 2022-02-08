#!/bin/bash

IPPOOL=(192.168.178.121 192.168.178.122 192.168.178.124 192.168.178.123)
# IPPOOL=(192.168.178.121 192.168.178.122 192.168.178.124)
PACKETS_TO_SEND=2
HOSTS_UP=()
PROJECT=revpi

CREATE_NEW=false

while [[ $# -gt 0 ]]
do
  key="$1"

  case $key in
    --new)
      CREATE_NEW=true
      shift
      ;;
  esac
done

# Check if tmux session is already running
tmux has-session -t $PROJECT 2>/dev/null

# If not running, create a new one
if [ $? != 0 ]; then
  tmux new-session -d -s $PROJECT
else
  # If running and should be created new, close all ssh connections, kill session and create new
  if $CREATE_NEW; then
    for pane_index in $(tmux list-panes -F '#P' -t $PROJECT); do
      tmux send-keys -t $PROJECT.$pane_index "exit" ENTER
    done
    tmux kill-session -t $PROJECT
    tmux new-session -d -s $PROJECT
  # otherwise, simply attach and exit
  else
    tmux attach -t $PROJECT
    exit 0
  fi
fi

# Check if the host is up by getting how many packets are received when pinging
# Append running hosts to a list
for IP in "${IPPOOL[@]}"
do
  PACKETS_RECEIVED=$(ping -4 -n -c $PACKETS_TO_SEND $IP | awk '/packets transmitted/ {print $4}')
  if [ $PACKETS_RECEIVED -eq $PACKETS_TO_SEND ]; then
    echo "HOST $IP is reachable!!"
    HOSTS_UP+=($IP)
  else
    echo "Host $IP is down...."
  fi
done

# Set tmux session as active one
tmux selectp -t $PROJECT

# Split the tmux session in as many windows as hosts are up
if [ ${#HOSTS_UP[@]} -eq 2 ]; then
  tmux split-window -h
elif [ ${#HOSTS_UP[@]} -eq 3 ]; then
  tmux split-window -h
  tmux select-pane -t 0
  tmux split-window -v
elif [ ${#HOSTS_UP[@]} -eq 4 ]; then
  tmux split-window -h
  tmux select-pane -t 0
  tmux split-window -v
  tmux select-pane -t 2
  tmux split-window -v
fi

pane_index=0
for IP in "${HOSTS_UP[@]}"
do
  tmux send -t $PROJECT.$pane_index "ssh pi@$IP" ENTER
  tmux send -t $PROJECT.$pane_index "cd ~/revpi/build" ENTER
  tmux send -t $PROJECT.$pane_index "clear" ENTER
  pane_index=$((pane_index+1))
done

tmux attach -t $PROJECT
