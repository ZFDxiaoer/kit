#!/bin/bash

old_path=`pwd`
cd `dirname $0`
path=`pwd`


$path/stop.sh

$path/michatroom 10300 &

cd $old_path 1>/dev/null

