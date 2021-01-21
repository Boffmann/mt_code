#!/bin/bash

IPPOOL=(192.168.178.121 192.168.178.122 192.168.178.123 192.168.178.124)
PACKETS_TO_SEND=2
PROJECT=revpi

RUN=false


deploy () {

  echo "Deploying on $1..."

  ssh pi@$1 "
    if [ ! -d /home/pi/$PROJECT ]; then
      mkdir /home/pi/$PROJECT
    fi
  "

  scp -r ./src/ pi@$1:/home/pi/$PROJECT
  scp ./CMakeLists.txt pi@$1:/home/pi/$PROJECT

  ssh pi@$1 "
    source /home/pi/ospl/opensplice-OSPL_V6_9_190925OSS_RELEASE/install/HDE/armv7l.linux-dev/release.com
    cd /home/pi/$PROJECT
    if [ -d \"build\" ]; then
      cd build
      make
    else
      mkdir build
      cd build
      cmake ..
      make
    fi
  "
}

run () {

  echo "RUNING worker on $1"

  ssh pi@$1 "
    source /home/pi/ospl/opensplice-OSPL_V6_9_190925OSS_RELEASE/install/HDE/armv7l.linux-dev/release.com
    cd /home/pi/$PROJECT/build
    tmux has-session -t $PROJECT 2>/dev/null

    if [ $? = 0 ]; then
      tmux new-session -d -s $PROJECT '/home/pi/$PROJECT/build/worker -w'
    else
      tmux kill-session -t $PROJECT
      tmux new-session -d -s $PROJECT '/home/pi/$PROJECT/build/worker -w'
    fi
  "
}

while [[ $# -gt 0 ]]
do
  key="$1"

  case $key in
    -r|--run)
      RUN=true
      shift
      ;;
  esac
done

for IP in "${IPPOOL[@]}"
do
  # Check if the host is up by getting how many packets are received when pinging
  PACKETS_RECEIVED=$(ping -4 -n -c $PACKETS_TO_SEND $IP | awk '/packets transmitted/ {print $4}')
  if [ $PACKETS_RECEIVED -eq $PACKETS_TO_SEND ]; then
    deploy $IP
    if $RUN; then
      run $IP
    fi
  else
    echo "Host $IP is down...."
  fi
done
