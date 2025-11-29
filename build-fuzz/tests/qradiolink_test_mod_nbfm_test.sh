#!/usr/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/haaken/github-projects/gr-qradiolink/tests
export GR_CONF_CONTROLPORT_ON=False
export PATH="/home/haaken/github-projects/gr-qradiolink/build-fuzz/tests":"$PATH"
export LD_LIBRARY_PATH="":$LD_LIBRARY_PATH
export PYTHONPATH=/home/haaken/github-projects/gr-qradiolink/build-fuzz/test_modules:$PYTHONPATH
qradiolink_test_mod_nbfm 
