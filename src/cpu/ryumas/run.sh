#!/bin/bash
cd "$(dirname "$0")"
exec ./ryumas "$@" 2> ryumas.err

