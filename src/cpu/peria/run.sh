#!/bin/bash
cd "$(dirname "$0")"
exec ./peria "$@" --pattern="book.txt" 2> peria.err

