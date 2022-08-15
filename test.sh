#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./mcc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 21 " 5 + 20 - 4 ;"
assert 12 "4*3;"
assert 3 "12/4;"
assert 16 "(5+3)*2;"
assert 10 "-10+20;"

assert 1 "1 == 1;"
assert 0 "1 != 1;"
assert 1 "1 < 2;"
assert 0 "1 > 2;"
assert 1 "1 <= 1;"
assert 1 "1 >= 1;"
assert 0 "1 <= -1;"

assert 16 "1+1;(5+3)*2;"

assert 3 "a=3;"
assert 7 "a=5*2-3;"
assert 5 "return 5;"
assert 12 "a=3; z=4; return a*z;"

echo OK
