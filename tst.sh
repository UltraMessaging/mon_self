#!/bin/sh
# tst.sh

if [ ! -f "lbm.sh" ]; then :
  echo "Must create 'lbm.sh' file (use 'lbm.sh.example' as guide)." >&2
  exit 1
fi

./bld.sh;  if [ $? -ne 0 ]; then exit 1; fi

. ./lbm.sh

cp lbtrm.cfg tst.cfg

echo "C mon_self lbtrm"
./mon_self;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

echo "J MonSelf lbtrm"
java --add-opens java.base/java.nio=ALL-UNNAMED $CP MonSelf;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

cp lbtru.cfg tst.cfg

echo "C mon_self lbtru"
./mon_self;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

echo "J MonSelf lbtru"
java --add-opens java.base/java.nio=ALL-UNNAMED $CP MonSelf;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

cp tcp.cfg tst.cfg

echo "C mon_self tcp"
./mon_self;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

echo "J MonSelf tcp"
java --add-opens java.base/java.nio=ALL-UNNAMED $CP MonSelf;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

cp lbtipc.cfg tst.cfg

echo "C mon_self lbtipc"
./mon_self;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

echo "J MonSelf lbtipc"
java --add-opens java.base/java.nio=ALL-UNNAMED $CP MonSelf;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

cp lbtsmx.cfg tst.cfg

echo "C mon_self lbtsmx"
./mon_self;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

echo "J MonSelf lbtsmx"
java --add-opens java.base/java.nio=ALL-UNNAMED $CP MonSelf;  if [ $? -ne 0 ]; then exit 1; fi
echo ""

