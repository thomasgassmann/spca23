#!/bin/bash

gcc -c ./level2/level2.s -o ./level2/level2.o
objdump -d ./level2/level2.o > ./level2/asm.s
