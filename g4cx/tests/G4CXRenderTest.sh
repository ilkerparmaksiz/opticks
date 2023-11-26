#!/bin/bash -l 
usage(){ cat << EOU
G4CXRenderTest.sh
===================

This is not currently expected to produce meaningful renders
because it lacks all the environment setup of g4cx/gxr.sh 

TODO: arrange C++ defaults in a way to produce something visible 
with most geometries

EOU
}

bin=G4CXRenderTest 
source $HOME/.opticks/GEOM/GEOM.sh 

vars="BASH_SOURCE GEOM bin"

defarg="info_run"
arg=${1:-$defarg}

if [ "${arg/info}" != "$arg" ]; then
   for var in $vars ; do printf "%20s : %s \n" "$var" "${!var}" ; done
fi

if [ "${arg/run}" != "$arg" ]; then
   $bin 
   [ $? -ne 0 ] && echo $BASH_SOURCE : run error && exit 1
fi

if [ "${arg/dbg}" != "$arg" ]; then
   dbg__ $bin 
   [ $? -ne 0 ] && echo $BASH_SOURCE : dbg error && exit 2
fi


exit 0 

