#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_filler --algorithm=peak "$@" 2> sample.err

