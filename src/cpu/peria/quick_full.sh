#!/bin/bash
cd "$(dirname "$0")"
exec ./quick_full "$@" 2> quick_full.err

