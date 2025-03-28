#!/bin/sh

sleep 5
export LD_LIBRARY_PATH=/opt/MasterOfZio:$LD_LIBRARY_PATH

sleep 10
#Start the application MasterOfZio
cd /opt/MasterOfZio
./MasterOfZio
