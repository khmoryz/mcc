#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3;}
int ret5() { return 5;}
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }

int add6(int a, int b, int c, int d, int e, int f) {
  return a+b+c+d+e+f;
}
EOF

assert() {
  expected="$1"
  input="$2"

  ./mcc "$input" > tmp.s
  cc -o tmp tmp.s tmp2.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "return 0;"
assert 42 "return 42;"
assert 21 "return 5+20-4;"
assert 21 "return  5 + 20 - 4 ;"
assert 12 "return 4*3;"
assert 3 "return 12/4;"
assert 16 "return (5+3)*2;"
assert 10 "return -10+20;"

assert 1 "return 1 == 1;"
assert 0 "return 1 != 1;"
assert 1 "return 1 < 2;"
assert 0 "return 1 > 2;"
assert 1 "return 1 <= 1;"
assert 1 "return 1 >= 1;"
assert 0 "return 1 <= -1;"

assert 3 "a=3; return a;"
assert 12 "a=3; z=4; return a*z;"

assert 6 "foo = 1; bar = 2 + 3; return foo + bar;"
assert 12 "foo1 = 3; BAR = 4; return foo1 * BAR;"

assert 3 "if (0) return 2; return 3;"
assert 3 "if (1-1) return 2; return 3;"
assert 2 "if (1) return 2; return 3;"
assert 2 "if (2-1) return 2; return 3;"
assert 3 "if (0) return 2; else return 3;"
assert 2 "if (1) return 2; else return 3;"

assert 3 "{1; {2;} return 3;}"

assert 10 "i=0; while(i<10) i=i+1; return i;"
assert 55 'i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j;'

assert 55 "i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;"
assert 3 "for (;;) return 3; return 5;"

assert 3 "return ret3();"
assert 5 "return ret5();"
assert 8 'return add(3, 5);'
assert 2 'return sub(5, 3);'
assert 21 'return add6(1,2,3,4,5,6);'

echo OK
