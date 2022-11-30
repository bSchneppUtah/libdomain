#!/bin/sh

mkdir -p ~/bgrt_results

../build/double-bgrt-ltr-5-pt | tail -n 2 >> ~/bgrt_results/FP64
../build/double-bgrt-ltr-7-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-ltr-9-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-ltr-13-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-ltr-25-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-ltr-27-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-ltr-125-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-ltr-poisson | tail -n 1 >> ~/bgrt_results/FP64
echo "\n" >> ~/bgrt_results/FP64
../build/double-bgrt-balanced-5-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-balanced-7-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-balanced-9-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-balanced-13-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-balanced-25-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-balanced-27-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-balanced-125-pt | tail -n 1 >> ~/bgrt_results/FP64
../build/double-bgrt-balanced-poisson | tail -n 1 >> ~/bgrt_results/FP64
