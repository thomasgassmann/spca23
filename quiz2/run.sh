#!/bin/bash

gcc -no-pie f.S g.S h.c test.c -o main && ./main
