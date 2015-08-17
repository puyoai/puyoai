#!/bin/bash
cd "$(dirname "$0")"
exec ./beam_search_ai --type=2dub --search_turns=10 --beam_width=200 --v=1 "$@" 2> quick_2dub.err

