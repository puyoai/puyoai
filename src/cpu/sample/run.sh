#!/bin/bash
cd "$(dirname "$0")"
exec ./sample "$@" 2> sample.err

