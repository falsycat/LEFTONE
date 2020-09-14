#!/bin/sh

# This script works only with memory-trace file generated in Linux.

file="memory-trace"
if [ ! -f $file ]; then
  echo no memory trace file found
  exit 1
fi

declare -A addrs

while read line; do
  IFS=" " read -r type addr1 trash1 addr2 trash2 <<< $line

  case "$type" in
  "new")
    addrs[$addr1]="allocated"
    ;;
  "resize")
    addrs[$addr1]="freed"
    addrs[$addr2]="allocated"
    ;;
  "delete")
    addrs[$addr1]="freed"
    ;;
  *) ;;
  esac
done < $file

for addr in "${!addrs[@]}"; do
  if [ "${addrs[$addr]}" == "allocated" ]; then
    echo $addr
  fi
done
