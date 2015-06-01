#!/bin/bash
cd "$(dirname "$0")"
exec ./control_tester --algorithm=right "$@" 2> sample.err
