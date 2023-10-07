#!/bin/bash

mkdir -p temp
python3 tools/stack2map.py --map src/kernel/enkel.map --input temp/stacktrace.txt