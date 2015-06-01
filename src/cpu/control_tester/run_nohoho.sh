#!/bin/bash
cd "$(dirname "$0")"
exec ./control_tester --algorithm=nohoho "$@" 2> sample.err

