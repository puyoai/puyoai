#!/bin/bash
cd "$(dirname "$0")"
exec ./beam_search_ai --type=2dub "$@" 2> quick_2dub.err

