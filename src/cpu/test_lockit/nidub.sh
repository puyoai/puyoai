#!/bin/bash
cd "$(dirname "$0")"
exec ./nidub "$@" 2> nidub.err

