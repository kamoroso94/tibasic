#!/usr/bin/env bash

mkdir -p build

if ! make ; then
  exit 1
fi

cp tibasic $HOME/bin
make clean
