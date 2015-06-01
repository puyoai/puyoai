#!/bin/bash
cd "$(dirname "$0")"
exec ./control_tester --algorithm=peak "$@" 2> sample.err

