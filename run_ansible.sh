#!/bin/bash

SHUTDOWN=false
DEPLOY=false
RUN=false
CANCEL=false
HELP=false
PROJECT=revpi

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -s|--shutdown)
      SHUTDOWN=true
      shift
      ;;
    -d|--deploy)
      DEPLOY=true
      shift
      ;;
    -r|--run)
      RUN=true
      shift
      ;;
    -c|--cancel)
      CANCEL=true
      shift
      ;;
    -h|--help)
      HELP=true
      shift
      ;;
  esac
done

if $HELP; then
  echo "-h|--help ------- Show this message"
  echo "-s|--shutdown --- Shut down all running Revolution Pis"
  echo "-r|--run -------- Start the program on all replicas"
  echo "-c|--cancel ----- Cancel the program on all replicas"
  echo "-d|--deploy ---.. Copy the new version and compile on replicas"

  exit 0
fi

if $DEPLOY; then
  echo "Copy and build new solution..."
  ansible-playbook ansible/deployOnSPS.yml
  exit 0
fi

if $CANCEL; then
  echo "Stopping the executions"
  NUM_PANES=$(tmux list-panes -t revpi | wc -l)
  NUM_PANES=`expr $NUM_PANES - 1`
  for pane_index in $( eval echo {0..$NUM_PANES}); do
    tmux send-keys -t $PROJECT.$pane_index C-c
  done
  exit 0
fi

if $RUN; then
  echo "Run the new solution"
  NUM_PANES=$(tmux list-panes -t revpi | wc -l)
  NUM_PANES=`expr $NUM_PANES - 1`
  for pane_index in $( eval echo {0..$NUM_PANES}); do
    tmux send -t $PROJECT.$pane_index "cd ~/revpi/build" ENTER
    tmux send -t $PROJECT.$pane_index "rm leader_election_duration.csv message_received.csv message_send.csv missed_election_timeouts.csv" ENTER
    tmux send -t $PROJECT.$pane_index "clear" ENTER
    tmux send -t $PROJECT.$pane_index "./replica $pane_index" ENTER
  done
  exit 0
fi

if $SHUTDOWN; then
  echo "Running ansible shutdown..."
  sleep 3
  ansible-playbook ansible/shutdown.yml

  exit 0
fi
