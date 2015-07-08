#!/bin/bash
cd "$(dirname "$0")"
export GLOG_v=1
exec ./mayah_cpu --num_threads=3 "$@" 2> run.err
