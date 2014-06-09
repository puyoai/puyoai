#!/bin/bash
cd "$(dirname "$0")"
exec ./gtr_main --slow_play "$@" 2> gtr.err
