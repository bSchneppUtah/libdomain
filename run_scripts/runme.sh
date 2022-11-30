#!/bin/sh

mkdir -p ~/bgrt_results

../build/bgrt-ltr-5-pt | tail -n 2 >> ~/bgrt_results/FP32
../build/bgrt-ltr-7-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-ltr-9-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-ltr-13-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-ltr-25-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-ltr-27-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-ltr-125-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-ltr-poisson | tail -n 1 >> ~/bgrt_results/FP32
echo "\n" >> ~/bgrt_results/FP32
../build/bgrt-balanced-5-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-balanced-7-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-balanced-9-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-balanced-13-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-balanced-25-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-balanced-27-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-balanced-125-pt | tail -n 1 >> ~/bgrt_results/FP32
../build/bgrt-balanced-poisson | tail -n 1 >> ~/bgrt_results/FP32
