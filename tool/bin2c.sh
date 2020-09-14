#!/bin/sh

set -e

name=`echo "$1" | sed -e 's/[^A-z0-9_]/_/g'`

hpath="$2.h"
cpath="$2.c"

data=`cat $in | xxd -i`
size=`echo $data | wc -w`

echo "const char $name[$size] = {$data};" > $cpath
echo "extern const char $name[$size];" > $hpath
