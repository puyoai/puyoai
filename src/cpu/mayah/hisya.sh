#!/bin/bash
cd "$(dirname "$0")"
exec ./mayah_cpu --from_wrapper --num_threads=3 \
     --feature="cpu/mayah/hisya_feature.toml" \
     --decision_book="cpu/mayah/hisya_decision.toml" \
     --pattern_book="cpu/mayah/hisya_pattern.toml" \
     "$@" 2> run.err
