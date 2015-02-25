#!/bin/bash
cd "$(dirname "$0")"
exec ./minim "$@" 2> minim.err

