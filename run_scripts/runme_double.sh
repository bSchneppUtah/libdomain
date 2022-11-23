#!/bin/sh

rm -rf ~/bgrt_results_double
mkdir ~/bgrt_results_double

../build/double-bgrt-ltr-5-pt | tail -n 1 > ~/bgrt_results_double/ltr_5_pt
../build/double-bgrt-balanced-5-pt | tail -n 1 > ~/bgrt_results_double/balanced_5_pt
../build/double-bgrt-ltr-7-pt | tail -n 1 > ~/bgrt_results_double/ltr_7_pt
../build/double-bgrt-balanced-7-pt | tail -n 1 > ~/bgrt_results_double/balanced_7_pt
../build/double-bgrt-ltr-9-pt | tail -n 1 > ~/bgrt_results_double/ltr_9_pt
../build/double-bgrt-balanced-9-pt | tail -n 1 > ~/bgrt_results_double/balanced_9_pt
../build/double-bgrt-ltr-13-pt | tail -n 1 > ~/bgrt_results_double/ltr_13_pt
../build/double-bgrt-balanced-13-pt | tail -n 1 > ~/bgrt_results_double/balanced_13_pt
../build/double-bgrt-ltr-25-pt | tail -n 1 > ~/bgrt_results_double/ltr_25_pt
../build/double-bgrt-balanced-25-pt | tail -n 1 > ~/bgrt_results_double/balanced_25_pt
../build/double-bgrt-ltr-27-pt | tail -n 1 > ~/bgrt_results_double/ltr_27_pt
../build/double-bgrt-balanced-27-pt | tail -n 1 > ~/bgrt_results_double/balanced_27_pt
../build/double-bgrt-ltr-125-pt | tail -n 1 > ~/bgrt_results_double/ltr_125_pt
../build/double-bgrt-balanced-125-pt | tail -n 1 > ~/bgrt_results_double/balanced_125_pt

../build/double-bgrt-ltr-poisson | tail -n 1 > ~/bgrt_results_double/ltr_poisson
../build/double-bgrt-balanced-poisson | tail -n 1 > ~/bgrt_results_double/balanced_poisson
