#!/bin/bash
# Runs the original and the new checker on every sample output (input 0) and compares the verdict line and exit code.
# The original checker is capped at 10 s CPU (the judge's limit, via ulimit -t); rc>=128 means it was killed.
cd "$(dirname "$0")"
printf "%-8s %-4s %-4s %-6s %s\n" sample rcOld rcNew same "new verdict"
for f in samples/raw_*.out; do t=${f#samples/raw_}; t=${t%.out}
  ( ulimit -t 10; exec ./chk_old samples/1.in $f samples/1.ans ) >/dev/null 2>old.txt; ro=$?
  ./chk_new samples/1.in $f samples/1.ans >/dev/null 2>new.txt; rn=$?
  vo=$(tail -1 old.txt); vn=$(tail -1 new.txt)
  if [ $ro -ge 128 ]; then s="oldTLE"; elif [ "$vo" == "$vn" ] && [ $ro -eq $rn ]; then s="yes"; else s="NO"; fi
  printf "%-8s %-4s %-4s %-6s %s\n" $t $ro $rn $s "${vn:0:110}"
done 2>/dev/null
rm -f old.txt new.txt
