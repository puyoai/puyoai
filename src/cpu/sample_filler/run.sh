#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_filler "$@" 2> sample.err

