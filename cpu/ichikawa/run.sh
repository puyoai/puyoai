#!/bin/bash
cd "$(dirname "$0")"
exec ./gtr_main "$@" 2> gtr.err
