#!/bin/bash
cd "$(dirname "$0")"
exec ./mayah_cpu "$@" 2> run.err
