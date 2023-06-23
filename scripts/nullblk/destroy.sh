#!/bin/bash

for nullblk in /dev/nullb*; do
  num=${nullblk#/dev/nullb} # 去掉前缀 /dev/nullb
  # if num is empty or is '*', ignore
  if [ -z "$num" -o "$num" = '*' ]; then
    continue
  fi
  echo "Destroying /dev/nullb$num"
  $(dirname "$0")/nullblk-zoned-remove.sh $num
done

echo "All nullblk devices destroyed"

# rmmod null_blk