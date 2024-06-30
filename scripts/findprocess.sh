#!/bin/sh

pid=$(pgrep $1)
echo $pid

echo $pid >> $2
