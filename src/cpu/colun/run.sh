#!/bin/bash
cd "$(dirname "$0")"
exec ./colun "$@" 2> colun.err

