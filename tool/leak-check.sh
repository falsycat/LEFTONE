#!/bin/bash

set -eu

file="memory-trace"
if [[ ! -f $file ]]; then
  echo no memory trace file found
  exit 1
fi

declare -A addrs

while IFS=$' \n' read -r type addr1 dummy1_ addr2 dummy2_; do
  case "$type" in
  "new")
    addrs[$addr1]=1
    ;;
  "resize")
    unset addrs[$addr1]
    ;;
  "delete")
    unset addrs[$addr1]
    ;;
  *)
    ;;
  esac
done < $file

for addr in "${!addrs[@]}"; do
  echo $addr
done
