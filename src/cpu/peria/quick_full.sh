#!/bin/bash
cd "$(dirname "$0")"
exec ./beam_search_ai --type=full "$@" 2> quick_full.err

