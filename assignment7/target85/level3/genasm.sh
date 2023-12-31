#!/bin/bash

gcc -c ./level3/level3.s -o ./level3/level3.o
objdump -d ./level3/level3.o > ./level3/asm.s
