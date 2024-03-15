#!/usr/bin/bash

ps aux | grep -ie gdb | awk '{print "kill -9 " $2}' | sh -x
ps aux | grep -ie qemu | awk '{print "kill -9 " $2}' | sh -x
ps aux | grep -ie python3 | awk '{print "kill -9 " $2}' | sh -x
