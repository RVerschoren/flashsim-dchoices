#! /bin/bash
for p in 0.01 0.05 0.1 0.2 0.5
  do for f in 0.01 0.05 0.10 0.20
    do
    #./SimMaxValid.exe DWF 32 100 0.1 $f $(printf %.2f $((1-$f))) 1 10 10000 5000 $p
    ./SimMaxValid.exe DWF 32 100 0.1 $f $(printf %.2f $( echo "(1-$f)" | bc -l)) 1 10 10000 5000 $p
    done
done
