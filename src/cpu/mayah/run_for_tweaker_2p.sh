#!/bin/bash
cd "$(dirname "$0")"
exec ./mayah_cpu --feature=feature.txt "$@" 2> run.err
