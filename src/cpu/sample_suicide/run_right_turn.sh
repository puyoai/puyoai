#!/bin/bash
cd "$(dirname "$0")"
exec ./sample_suicide --right_turn "$@" 2> sample.err
