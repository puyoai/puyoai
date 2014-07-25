#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_filter "$@" 2> sample.err

