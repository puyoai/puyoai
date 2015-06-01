#!/bin/bash
cd "$(dirname "$0")"
exec ./control_tester --algorithm=quickturn "$@" 2> sample.err

