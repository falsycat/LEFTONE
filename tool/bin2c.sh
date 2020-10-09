#!/bin/bash

set -eu

if [[ -z ${1+x} || -z ${2+x} ]]; then
  echo "usage: cat <file> | ./bin2c.sh <identifier name> <output name>"
  exit 1
fi

name=$(sed -e 's/[^A-z0-9_]/_/g' <<< "$1")

hpath="$2.h"
cpath="$2.c"

data=$(cat - | xxd -i)
size=$(wc -w <<< "$data")

echo "const char $name[$size] = {$data};" > $cpath
echo "extern const char $name[$size];" > $hpath
