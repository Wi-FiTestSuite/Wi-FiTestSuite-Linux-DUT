#!/bin/sh

#
# Copyright (c) 2016 Wi-Fi Alliance
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
# 
#
# $1 is tmp file name
# $2 is the interface name

echo -n "dhcpcli=" > $1 ; ps ax | grep dhc | cut -f2 -d: | cut -f2 -d' ' | grep dhclient >> $1
echo ' ' >> $1
echo -n "mac=" >> $1; ifconfig $2 | grep HWaddr | cut -f3 -dr >> $1 
echo -n "ipaddr=" >> $1; ifconfig $2 | grep "inet addr" | cut -f2 -d: >> $1 
echo -n "bcast=" >> $1; ifconfig $2 | grep "inet addr" | cut -f3 -d: >> $1 
echo -n "mask=" >> $1; ifconfig $2 | grep "inet addr" | cut -f4 -d: >> $1 
grep nameserver /etc/resolv.conf >> $1 
