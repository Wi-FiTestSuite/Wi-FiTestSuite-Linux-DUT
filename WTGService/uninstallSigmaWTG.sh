#!/bin/sh
# Uninstall the SigmaWTG service to system startup


sh $PWD/SigmaWTGService.sh stop
rm -f /usr/bin/SigmaWTG /usr/bin/wfa_dut /usr/bin/wfa_ca
rm -f /etc/SigmaWTG.conf
rm -f /etc/rc.local
