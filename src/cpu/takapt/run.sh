#!/bin/bash
cd "$(dirname "$0")"
exec ./takapt "$@" 2> takapt.err
