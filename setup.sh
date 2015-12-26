#!/bin/bash
make lib
make
export PATH="$PATH:$PWD/bin"
export DYLD_LIBRARY_PATH="$DYLD_LIBRARY_PATH:$PWD/bin"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PWD/bin"

