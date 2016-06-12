#!/bin/bash
cd "$(dirname "$0")"
exec ./yukina_cpu --from_wrapper --num_threads=3 "$@" 2> run.err
