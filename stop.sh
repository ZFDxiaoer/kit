#!/bin/bash

old_path=`pwd`
cd `dirname $0`
path=`pwd`
killall $path/michatroom 2>/dev/null

cd $old_path 1>/dev/null

