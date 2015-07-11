#!/bin/bash
cd "$(dirname "$0")"
exec ./niina --type="rendaS9" "$@" 2> rendaS9.err
