#!/bin/bash
cd "$(dirname "$0")"
exec ./yukina_cpu --from_wrapper --num_threads=28 "$@" 2> run.err
