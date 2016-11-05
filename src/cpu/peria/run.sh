#!/bin/bash
cd "$(dirname "$0")"
exec ./peria "$@" 2> peria.err

