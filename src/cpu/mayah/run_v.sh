#!/bin/bash
cd "$(dirname "$0")"
export GLOG_v=1
exec ./mayah_cpu --use_advanced_next --num_threads=3 "$@" 2> run.err
