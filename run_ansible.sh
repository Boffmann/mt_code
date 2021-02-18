#!/bin/bash

SHUTDOWN=false
DEPLOY=false
RUN=false
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
    -h|--help)
      HELP=true
      shift
      ;;
  esac
done

if $HELP; then
  echo "-h|--help ------- Show this message"
  echo "-s|--shutdown --- Shut down all running Revolution Pis"

  exit 0
fi

if $DEPLOY; then
  echo "Copy and build new solution..."
  ansible-playbook ansible/deployOnSPS.yml
  exit 0
fi

if $RUN; then
  echo "Run the new solution"
  NUM_PANES=$(tmux list-panes -t revpi | wc -l)
  NUM_PANES=`expr $NUM_PANES - 1`
  for pane_index in $( eval echo {0..$NUM_PANES}); do
    tmux send -t $PROJECT.$pane_index "cd ~/revpi/build" ENTER
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
