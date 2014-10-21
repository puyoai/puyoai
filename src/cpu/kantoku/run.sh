#!/bin/bash
cd "$(dirname "$0")"
exec ./kantoku "$@" 2> run.err
