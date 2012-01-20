#!/bin/sh
# Install the SigmaWTG service to system startup


sh $PWD/SigmaWTGService.sh stop
cp  SigmaWTG ../dut/wfa_dut ../ca/wfa_ca /usr/bin
cp  SigmaWTG.conf /etc
if [ ! -d /SIGMA_WTG ] 
then
	mkdir /SIGMA_WTG
fi	

if [ ! -d /SIGMA_WTGv2 ]
then
        mkdir /SIGMA_WTGv2
fi

cp  ../scripts/* /usr/bin/
cp  ../scripts/* /usr/sbin/
cp  ../scripts/* /usr/local/bin/
cp  ../scripts/* /usr/local/sbin/
chmod 777 /usr/local/sbin/*
rm -f /etc/rc.local
ln -s $PWD/SigmaWTGService.sh /etc/rc.local
sh $PWD/SigmaWTGService.sh start

