#!/bin/bash
cd "$(dirname "$0")"
exec ./rendaGS9 "$@" 2> rendaGS9.err
