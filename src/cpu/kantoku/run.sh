#!/bin/bash
cd "$(dirname "$0")"
exec ./kantoku --change_if_beated=false --change_random=true "$@" 2> run.err
