#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_filler --peak=true "$@" 2> sample.err

