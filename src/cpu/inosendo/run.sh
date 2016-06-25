#!/bin/bash
cd "$(dirname "$0")"
exec ./inosendo "$@" 2> inosendo.err

