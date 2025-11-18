#!/bin/bash
################################################################################
# DEFINE INSTALL OS PATCHES FUNCTIONS
################################################################################

function fixup_dtoverlay_linking (){
	#####################################################################
    echo "--------------------------------------------------------------"
    echo " Fixing DTOVERLAY Linking                 "
    echo "--------------------------------------------------------------"
    #####################################################################
	apt install cmake device-tree-compiler libfdt-dev --assume-yes --fix-missing  
   wget https://github.com/raspberrypi/utils/archive/refs/heads/master.zip
    unzip master.zip;
    cd utils-master;
    cmake .;
    make;
    make install;
    cd ..;
    rm -rf utils-master;
    rm master.zip
    echo "Completed"
}