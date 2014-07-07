#!/bin/bash
cd "$(dirname "$0")"
exec ./mayah_cpu --log_max_score=true --feature=feature.txt "$@" 2>> run2.err
