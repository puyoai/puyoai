#!/bin/bash
cd "$(dirname "$0")"
export GLOG_v=1
exec ./mayah_cpu "$@" 2> run.err
