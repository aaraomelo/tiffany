#!/bin/bash
ERG=/c/Users/jssim/aarao/tiffany/tools/bin/erg_new.exe
BIN=/c/Users/jssim/aarao/tiffany/banco/sigma.bin
PASS=0
FAIL=0
for x in $(seq 0 255); do
  $ERG zera /tmp/sigma_mem.dat 20 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 1 $x 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 0 0 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 2 0 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 10 1 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 11 2 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 12 4 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 13 8 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 14 16 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 15 32 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 16 64 0 > /dev/null 2>&1
  $ERG poe /tmp/sigma_mem.dat 17 128 0 > /dev/null 2>&1
  $ERG corre $BIN /tmp/sigma_mem.dat > /dev/null 2>&1
  result=$($ERG ve /tmp/sigma_mem.dat 2 2>&1 | awk '{print $1}')
  expected=$(python -c "print(bin($x).count('1'))")
  if [ "$result" != "$expected" ]; then
    echo "FAIL x=$x expected=$expected actual=$result"
    FAIL=$((FAIL+1))
  else
    PASS=$((PASS+1))
  fi
done
echo "PASS=$PASS FAIL=$FAIL"
