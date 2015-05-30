#!/bin/sh

#
# Copyright (c) 2014 Wi-Fi Alliance
# 
# Permission to use, copy, modify, and/or distribute this software for any 
# purpose with or without fee is hereby granted, provided that the above 
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY 
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
# RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
# USE OR PERFORMANCE OF THIS SOFTWARE.
#

# Install the SigmaWTG service to system startup


sh $PWD/SigmaWTGService.sh stop
mkdir /etc/WfaEndpoint
cp -f ../wfa_cli.txt /etc/WfaEndpoint/.
cp -f ../scripts/* /usr/local/sbin/
cp  SigmaWTG ../dut/wfa_dut ../ca/wfa_ca /usr/bin
cp ../console_src/wfa_con  /usr/bin/.
cp  SigmaWTG.conf /etc
if [ ! -d /SIGMA_WTGv2 ] 
then
	mkdir /SIGMA_WTGv2
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

