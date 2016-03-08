#!/bin/bash
cd "$(dirname "$0")"
exec ./peria "$@" --pattern="book.txt" --type=1 2> pai.err

