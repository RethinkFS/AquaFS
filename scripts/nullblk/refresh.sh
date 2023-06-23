#!/bin/bash

if [ $# != 1 ]; then
  echo "Usage: $0 <nr nullblks>"
  exit 1
fi

$(dirname "$0")/destroy.sh

$(dirname "$0")/create.sh $1 512