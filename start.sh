#!/bin/bash
red="\033[0;31m"
yellow="\033[1;33m"
green="\033[1;32m"
reset="\033[0m"

if [ $# -ne 2 ]
then
  echo -e "\n Use ./start [port] [numberOfClients] \n"
  exit 1
fi

port=$1
clients=$2

printf "${yellow}Starting the $clients clients:${reset}\n"

COUNTER=0
while [  $COUNTER -lt $clients  ]; do
  sleep 1
  echo -e "\n --- New client --- \n"
  ./client localhost $port &
  let COUNTER=COUNTER+1
done
