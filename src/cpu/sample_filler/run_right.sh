#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_filler --algorithm=right "$@" 2> sample.err
