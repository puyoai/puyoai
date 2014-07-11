#!/bin/bash
cd "$(dirname "$0")"
exec ./yamaguchi "$@" 2> yamaguchi.err

