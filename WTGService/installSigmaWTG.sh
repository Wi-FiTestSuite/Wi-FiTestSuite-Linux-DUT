#!/bin/sh
# Install the SigmaWTG service to system startup


sh $PWD/SigmaWTGService.sh stop
cp  SigmaWTG ../dut/wfa_dut ../ca/wfa_ca /usr/bin
cp  SigmaWTG.conf /etc
rm -f /etc/rc.local
ln -s $PWD/SigmaWTGService.sh /etc/rc.local
sh $PWD/SigmaWTGService.sh start

