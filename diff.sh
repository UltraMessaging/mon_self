#!/bin/sh
# diff.sh
# Assumes "tst.cfg" is already set up.

if [ ! -f "lbm.sh" ]; then :
  echo "Must create 'lbm.sh' file (use 'lbm.sh.example' as guide)." >&2
  exit 1
fi

./bld.sh;  if [ $? -ne 0 ]; then exit 1; fi

. ./lbm.sh

echo ""
cat tst.cfg

echo "mon_self"
./mon_self 2>&1 | tee raw.c.log | sed 's/source=[^,]*,/source=x,/;s/sent=[0-9]*/sent=x/;s/rcved=[0-9]*/rcved=x/;/Core-/d' >c.log;  if [ $? -ne 0 ]; then exit 1; fi

echo "MonSelf"
java --add-opens java.base/java.nio=ALL-UNNAMED $CP MonSelf 2>&1 | tee raw.j.log | sed 's/source=[^,]*,/source=x,/;s/sent=[0-9]*/sent=x/;s/rcved=[0-9]*/rcved=x/;/Core-/d' >j.log;  if [ $? -ne 0 ]; then exit 1; fi

diff c.log j.log >diff.log
wc c.log j.log diff.log
