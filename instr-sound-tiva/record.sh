#!/bin/sh
INFILE=${3:-/dev/ttyACM0}

stty -F ${INFILE} 921600
dd if=${INFILE} of=/tmp/rec.raw count=${1}

sox -r 22050 -e unsigned -b 8 -c 1 /tmp/rec.raw ${2}
rm /tmp/rec.raw
