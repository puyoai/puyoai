#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_rensa "$@" 2> sample.err

