#!/bin/bash
cd "$(dirname "$0")"
exec ./shinyak_cpu -log_dir=. "$@" 2> run.err
