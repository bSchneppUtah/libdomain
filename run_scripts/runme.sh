#!/bin/sh

rm -rf ~/bgrt_results
mkdir ~/bgrt_results

../build/bgrt-ltr-5-pt | tail -n 1 > ~/bgrt_results/ltr_5_pt
../build/bgrt-balanced-5-pt | tail -n 1 > ~/bgrt_results/balanced_5_pt
../build/bgrt-ltr-7-pt | tail -n 1 > ~/bgrt_results/ltr_7_pt
../build/bgrt-balanced-7-pt | tail -n 1 > ~/bgrt_results/balanced_7_pt
../build/bgrt-ltr-9-pt | tail -n 1 > ~/bgrt_results/ltr_9_pt
../build/bgrt-balanced-9-pt | tail -n 1 > ~/bgrt_results/balanced_9_pt
../build/bgrt-ltr-13-pt | tail -n 1 > ~/bgrt_results/ltr_13_pt
../build/bgrt-balanced-13-pt | tail -n 1 > ~/bgrt_results/balanced_13_pt
../build/bgrt-ltr-25-pt | tail -n 1 > ~/bgrt_results/ltr_25_pt
../build/bgrt-balanced-25-pt | tail -n 1 > ~/bgrt_results/balanced_25_pt
../build/bgrt-ltr-27-pt | tail -n 1 > ~/bgrt_results/ltr_27_pt
../build/bgrt-balanced-27-pt | tail -n 1 > ~/bgrt_results/balanced_27_pt
../build/bgrt-ltr-125-pt | tail -n 1 > ~/bgrt_results/ltr_125_pt
../build/bgrt-balanced-125-pt | tail -n 1 > ~/bgrt_results/balanced_125_pt

../build/bgrt-ltr-poisson | tail -n 1 > ~/bgrt_results/ltr_poisson
../build/bgrt-balanced-poisson | tail -n 1 > ~/bgrt_results/balanced_poisson
