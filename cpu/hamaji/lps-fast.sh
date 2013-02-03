#!/bin/sh

cd $(dirname $0)
./lps --handle_opponent_grounded=0 --use_next_next=0
