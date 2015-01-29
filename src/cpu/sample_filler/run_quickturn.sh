#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_filler --algorithm=quickturn "$@" 2> sample.err

