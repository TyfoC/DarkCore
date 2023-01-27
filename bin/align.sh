#!/bin/bash
size=$1
value=$(($2-1))
invValue=$((~$value))
incValue=$(($size + $value))
echo $(($incValue & $invValue))