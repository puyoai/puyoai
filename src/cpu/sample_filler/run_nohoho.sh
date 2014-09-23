#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_filler --algorithm=nohoho "$@" 2> sample.err

