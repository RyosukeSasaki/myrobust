#!/bin/sh
cd recv
md5sum $(find . -type f) | tee check.md5