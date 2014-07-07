#!/bin/sh

set -eux

function sum_max_score() {
    local filename=$1
    cat "${filename}" | grep -e 'max score' | cut -d ' ' -f 4 | xargs echo | sed -e 's/ / + /g' | xargs expr
}

function run_tweak_and_duel() {
    rm ./cpu/mayah/run2.err
    ./cpu/mayah/tweaker --feature=./cpu/mayah/feature.txt
    for i in {0..99}; do
        echo "seed $i"
        ./duel/duel --num_duel=1 --realtime=false --no_timeout=true --seed="$i" --use_gui=false --use_cui=false --httpd=false -- cpu/mayah/run_for_tweaker_1p.sh cpu/mayah/run_for_tweaker_2p.sh
    done

    sum_max_score ./cpu/mayah/run2.err
}

function run_one_time_loop() {
    cp ./cpu/mayah/best_feature.txt ./cpu/mayah/feature.txt
    score=`run_tweak_and_duel`

    if [ -e ./cpu/mayah/best_score.txt ]; then
        bestscore=`cat ./cpu/mayah/best_score.txt`
    else
        bestscore=0
    fi

    echo "score = $score"
    echo "current bestscore = $bestscore"

    if [ $score -gt $bestscore ]; then
        echo $score > ./cpu/mayah/best_score.txt
        cp ./cpu/mayah/feature.txt ./cpu/mayah/best_feature.txt
    else
        cp ./cpu/mayah/best_feature.txt ./cpu/mayah/feature.txt
    fi
}

run_one_time_loop
