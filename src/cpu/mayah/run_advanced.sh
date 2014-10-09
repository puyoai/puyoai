#!/bin/bash
cd "$(dirname "$0")"
exec ./mayah_cpu --use_advanced_next=true "$@" 2> run.err
