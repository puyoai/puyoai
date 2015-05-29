#!/bin/bash
cd "$(dirname "$0")"
exec ./rendaS9 "$@" 2> rendaS9.err

