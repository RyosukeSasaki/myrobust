#!/bin/sh
rm ./data/*

for i in `seq 0 999`
do
    cat /dev/urandom | base64 | head -c 102400 > data/data$i
done

cd data
md5sum $(find . -type f) | tee check.md5