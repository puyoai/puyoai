#!/bin/bash
cd "$(dirname "$0")"
exec ./peria "$@" --dynamic_pattern="dynamic_book.txt" 2> peria.err

