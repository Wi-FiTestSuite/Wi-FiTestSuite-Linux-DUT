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

echo "DIR Configuration"
if [ ! -d /usr/tmp ]
then
	mkdir /usr/tmp
fi
rm -f /usr/tmp/.identity

echo PC Endpoint > /usr/tmp/.identity
echo 9003 >> /usr/tmp/.identity
echo END >> /usr/tmp/.identity
cp  ../scripts/* /usr/bin/
cp  ../scripts/* /usr/sbin/
cp  ../scripts/* /usr/local/bin/
cp  ../scripts/* /usr/local/sbin/
chmod 777 /usr/local/sbin/*
rm -f /etc/rc.local
rm -f /tmp/*.txt
if [ $1 == "VE" ]
then
	cp startPCE_VE.sh /usr/bin
	echo "in VE"
	ln -s $PWD/SigmaWTGService_VE.sh /etc/rc.local
	sh $PWD/SigmaWTGService_VE.sh start
else
	ln -s $PWD/SigmaWTGService.sh /etc/rc.local
	sh $PWD/SigmaWTGService.sh start
fi

