#!/bin/bash
cd "$(dirname "$0")"
exec ./shinyak_cpu "$@" 2> run.err
