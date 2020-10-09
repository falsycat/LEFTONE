#!/bin/bash

set -eu

DEFAULT_SERIALIZER="${DEFAULT_SERIALIZER:-}"
DEFAULT_DESERIALIZER="${DEFAULT_DESERIALIZER:-}"

preproc=$(cat << EOS

s|^\s*(.*)\s*$|\1|

/^\/\*\s*CRIAL\s*$/, /^\*\/$/ ! d

/^\/\*\s*CRIAL\s*$/ d
/^\*\/$/            d

EOS
)

seq=
part_serializer="$DEFAULT_SERIALIZER"
part_deserializer="$DEFAULT_DESERIALIZER"

serializer=
deserializer=

last_name=
count=0

add_property() {
  name=$(awk -F ' ' '{print $1}' <<< "$1")
  code=$(awk -F ' ' '{print $1}' <<< "$2")

  local s="$part_serializer"
  local d="$part_deserializer"

  s=${s//\$name/$name}
  d=${d//\$name/$name}
  s=${s//\$code/$code}
  d=${d//\$code/$code}

  serializer+="do { $s } while (0);"
  deserializer+="do { $d } while (0);"

  (( ++count ))
}

while IFS=$'\n' read line; do
  if [[ ! $line ]]; then continue; fi

  case $seq in
  serializer)
    if [[ $line == "END" ]]; then
      seq=
    else
      part_serializer+="$line"
    fi
    ;;
  deserializer)
    if [[ $line == "END" ]]; then
      seq=
    else
      part_deserializer+="$line"
    fi
    ;;
  *)
    if [[ $line =~ ^# ]]; then
      :
    elif [[ $line =~ ^SERIALIZER_BEGIN$ ]]; then
      seq="serializer"
      part_serializer=
    elif [[ $line =~ ^DESERIALIZER_BEGIN$ ]]; then
      seq="deserializer"
      part_deserializer=
    elif [[ $line =~ ^PROPERTY[[:blank:]](.*)=(.*)$ ]]; then
      add_property "${BASH_REMATCH[1]}" "${BASH_REMATCH[2]}"
    elif [[ $line =~ ^PROPERTY[[:blank:]](.*)$ ]]; then
      add_property "${BASH_REMATCH[1]}" "${BASH_REMATCH[1]}"
    else
      echo crial syntax error
      exit 1
    fi
    ;;
  esac
done < <(cat - | sed -r "$preproc")

echo "#define CRIAL_PROPERTY_COUNT_ $count"
echo "#define CRIAL_SERIALIZER_ do { $serializer } while (0)"
echo "#define CRIAL_DESERIALIZER_ do { $deserializer } while (0)"
