#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_suicide "$@" 2> sample.err

