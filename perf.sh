#!/bin/sh

perf stat -d -e cycles,bus-cycles,instructions,branches,branch-misses,cache-references,cache-misses $@
