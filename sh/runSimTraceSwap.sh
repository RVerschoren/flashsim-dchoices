#! /bin/bash
for p in 0.05 0.01 0.005 0.001; do ./SimTraceSwap.exe DWF 32 100 0.1 0.01 11 40 5000 245792 Booting_log176.csv 1 $p; done
