#!/usr/bin/env bash

mkdir -p build

if ! make ; then
  exit 1
fi

cp -t /usr/local/bin tibasic
make clean
