#!/bin/bash

sudo umount /tmp/test; ps -aef | grep dupfs | awk {'print $2'} | xargs kill -9; qmake && make && bin/dupfs /tmp/test
