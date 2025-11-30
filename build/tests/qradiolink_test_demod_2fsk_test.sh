#!/usr/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/haaken/github-projects/authenticated-repeater-control/tests
export GR_CONF_CONTROLPORT_ON=False
export PATH="/home/haaken/github-projects/authenticated-repeater-control/build/tests":"$PATH"
export LD_LIBRARY_PATH="":$LD_LIBRARY_PATH
export PYTHONPATH=/home/haaken/github-projects/authenticated-repeater-control/build/test_modules:$PYTHONPATH
qradiolink_test_demod_2fsk 
