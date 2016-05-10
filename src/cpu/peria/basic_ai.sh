#!/bin/bash
cd "$(dirname "$0")"
exec ./peria "$@" --pattern="book.txt" --type=2 2> peria_basic.err

