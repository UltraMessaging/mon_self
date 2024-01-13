#!/bin/sh
# tst.sh

if [ ! -f "lbm.sh" ]; then :
  echo "Must create 'lbm.sh' file (use 'lbm.sh.example' as guide)." >&2
  exit 1
fi

./bld.sh;  if [ $? -ne 0 ]; then exit 1; fi

. ./lbm.sh

echo "mon_self"
./mon_self;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

echo "MonSelf"
java --add-opens java.base/java.nio=ALL-UNNAMED $CP MonSelf;  if [ $? -ne 0 ]; then exit 1; fi
echo ""
