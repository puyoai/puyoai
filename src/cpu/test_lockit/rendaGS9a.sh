#!/bin/bash
cd "$(dirname "$0")"
exec ./niina --type="rendaGS9a" "$@" 2> rendaGS9a.err
