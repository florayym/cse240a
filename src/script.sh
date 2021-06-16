#!/bin/bash

cd /home/flora/cse240a/src

traces=(fp_1 fp_2 int_1 int_2 mm_1 mm_2)

echo "===========static==========="
for trace in "${traces[@]}"; do
    echo "${trace}"
    bunzip2 -kc ../traces/${trace}.bz2 | ./predictor
done

echo "===========gshare==========="
max_ghistory=28
for trace in "${traces[@]}"; do
    echo "${trace}"
    for ((i=1;i<=${max_ghistory};i++)) do
        bunzip2 -kc ../traces/${trace}.bz2 | ./predictor --gshare:${i}
    done
done

echo "===========tournament==========="
max_ghistory=9
max_lhistory=10
max_index=10
for trace in "${traces[@]}"; do
    echo "${trace}"
    for ((i=1;i<=${max_ghistory};i++)) do
        for ((j=1;j<=${max_lhistory};j++)) do
            for ((k=1;k<=${max_index};k++)) do
                bunzip2 -kc ../traces/${trace}.bz2 | ./predictor --tournament:${i}:${j}:${k}
            done
        done
    done
done

echo "===========custom==========="
max_ghistory=28
for trace in "${traces[@]}"; do
    echo "${trace}"
    for ((i=1;i<=${max_ghistory};i++)) do
        bunzip2 -kc ../traces/${trace}.bz2 | ./predictor --custom:${i}
    done
done