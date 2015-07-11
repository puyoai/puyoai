#!/bin/bash
cd "$(dirname "$0")"
exec ./niina --type="nidub" "$@" 2> nidub.err
