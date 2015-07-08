#!/bin/bash
cd "$(dirname "$0")"
exec ./kantoku --change_if_beated=false "$@" 2> run.err
