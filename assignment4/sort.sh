#!/bin/bash

gcc -no-pie sort.S sort.c -o sort && ./sort
