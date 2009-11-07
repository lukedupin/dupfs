#!/bin/bash

sudo umount /DevSync; ps -aef | grep dupfs | awk {'print $2'} | xargs kill -9; qmake && make && bin/dupfs
