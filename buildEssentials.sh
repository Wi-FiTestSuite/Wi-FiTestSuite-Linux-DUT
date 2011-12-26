#!/bin/bash
#To install the build essentials

cp /etc/apt/sources.list /etc/apt/sources.list.orig
cp -f sources.list /etc/apt

apt-get update

echo y > yfile
apt-get install build-essential < yfile

rm yfile
