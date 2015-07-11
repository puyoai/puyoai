#!/bin/bash
cd "$(dirname "$0")"
exec ./niina --type="rendaGS9" "$@" 2> rendaGS9.err
