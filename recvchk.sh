#!/bin/sh
cd recv
md5sum $(find . -type f) | tee check.md5

diff check.md5 ../data/check.md5