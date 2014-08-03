#!/bin/bash
cd "$(dirname "$0")"
exec ./tzik "$@" 2> tzik.err
