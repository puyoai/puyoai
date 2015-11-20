#!/bin/bash
cd "$(dirname "$0")"
exec ./mayah_cpu --from_wrapper --num_threads=3 "$@" 2> run.err
