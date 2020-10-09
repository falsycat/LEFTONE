#!/bin/bash

set -eu

if [[ -z ${1+x} || -z ${2+x} ]]; then
  echo "usage: cat <file> | ./benum.sh <dst path without extension> <header path>"
  exit 1
fi

DSTPATH="$1"
DSTDIR=$(dirname "$DSTPATH")

HEADERPATH="$2"

if [[ $DSTPATH =~ /$ ]]; then
  echo "destination path must be a filename without extension" >&2
  exit 1
fi
if [[ ! -d $DSTDIR ]]; then
  echo "no such directory: $DSTDIR" >&2
  exit 1
fi

preprocess=$(cat << EOS

s|^\s*||
s|\s*$||

/^\/\* *BENUM  *BEGIN.*\*\/$/,/^\/\* *BENUM  *END *\*\/$/ ! d
/^\/\* *BENUM  *BEGIN.*\*\/$/ {
  s|^\/\* *BENUM  *BEGIN *([^ ]*) *\*\/$|^\1|
  p
  s|.||g
  x
  b
}
/^\/\* *BENUM  *END *\*\/$/ {
  x
  s|\s||g
  s|,|\\n|g
  s|\\n\\n*|\\n|g
  s|^\\n||
  s|\\n$||
  p; b
}

H

EOS
)

header=$(cat << EOS
#include <stdbool.h>
#include <stddef.h>
EOS
)
src=$(cat << EOS
#include "$HEADERPATH"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
EOS
)

name=
NAME=

list=()

output_header() {
  if [[ -z $name ]]; then return; fi

  cat << EOS

#define ${NAME}_EACH(PROC) do {  \\
  $(for item in ${list[@]}; do
    echo "PROC($item, ${item,,});  \\"
  done)
} while (0)

#define ${NAME}_COUNT ${#list[@]}

const char* ${name}_stringify(${name}_t v);
bool ${name}_unstringify(${name}_t* v, const char* str, size_t len);

EOS
}
output_source() {
  if [[ -z $name ]]; then return; fi

  cat << EOS

const char* ${name}_stringify(${name}_t v) {
  switch (v) {
  $(for item in ${list[@]}; do
    echo "case ${NAME}_$item: return \"$item\";"
  done)
  }
  assert(false);
  return NULL;
}
bool ${name}_unstringify(${name}_t* v, const char* str, size_t len) {
  assert(v != NULL);
  assert(str != NULL || len == 0);
  $(for item in ${list[@]}; do
    echo "if (len == ${#item} && strncmp(str, \"$item\", len) == 0) {"
    echo "  *v = ${NAME}_$item;"
    echo "  return true;"
    echo "}"
  done)
  return false;
}

EOS
}
output() {
  header+=$(output_header)
  src+=$(output_source)
}

while read line; do
  if [[ $line =~ ^\^(.*)$ ]]; then
    output
    name="${BASH_REMATCH[1]}"
    name="${name,,}"
    NAME="${name^^}"
    list=()
  else
    if [[ ! $line =~ ^${NAME}_(.*)$ ]]; then
      echo invalid naming rule: "$line" >&2
      exit 1
    fi
    list+=(${BASH_REMATCH[1]})
  fi
done < <(cat | sed -r -n "$preprocess")
output

echo "$header" > "$DSTPATH.h"
echo "$src"    > "$DSTPATH.c"
