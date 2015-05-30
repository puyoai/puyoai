#!/bin/bash
cd "$(dirname "$0")"
exec ./yuricat "$@" 2> yuricat.err

