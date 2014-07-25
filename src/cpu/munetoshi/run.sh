#!/bin/bash
cd "$(dirname "$0")"
exec ./munetoshi "$@" 2> munetoshi.err

