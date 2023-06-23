#!/bin/bash

if [ $# != 2 ]; then
  echo "Usage: $0 <nr nullblks> <nr seq zones>"
  exit 1
fi

for ((i=0; i<$1; i++)); do
  echo "Creating nullblk $i with args: blksz=512 zonesz=16MiB conv=0 seq=$2"
  $(dirname "$0")/nullblk-zoned.sh 512 16 0 $2
done