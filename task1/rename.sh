#!/bin/bash

suffix=.orig
# Found this in manual: -exec command {} +
find -name "*.c" -exec cp {} {}$suffix \;
