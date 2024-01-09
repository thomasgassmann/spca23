#!/bin/bash

cat ./level5/level5.txt | ./hex2raw > out.raw && gdb -ex 'b *0x401e00' -ex 'run < out.raw' ./rtarget -q
