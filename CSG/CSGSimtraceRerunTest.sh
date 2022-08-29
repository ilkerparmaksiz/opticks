#!/bin/bash -l 
usage(){ cat << EOU
CSGSimtraceRerunTest.sh
================================

For CSGSimtraceRerunTest to provide full debug info it is necessary to 
recompile the CSG package with two non-standard preprocessor macros, 
that are not intended to ever be committed::

    DEBUG
    DEBUG_RECORD


::

    c
    ./CSGSimtraceRerunTest.sh 
    ./CSGSimtraceRerunTest.sh info
    ./CSGSimtraceRerunTest.sh run 
    ./CSGSimtraceRerunTest.sh ana

EOU
}


arg=${1:-run}

bin=CSGSimtraceRerunTest
log=$bin.log
source $(dirname $BASH_SOURCE)/../bin/COMMON.sh 

UGEOMDIR=${GEOMDIR//$HOME\/}
BASE=$GEOMDIR/$bin
UBASE=${BASE//$HOME\/}    # UBASE relative to HOME to handle rsync between different HOME
FOLD=$BASE/ALL            # corresponds SEvt::save() with SEvt::SetReldir("ALL")

T_FOLD=${FOLD/$bin/G4CXSimtraceTest}

if [ "info" == "$arg" ]; then
    vars="BASH_SOURCE arg bin GEOM GEOMDIR UGEOMDIR BASE UBASE FOLD T_FOLD"
    for var in $vars ; do printf "%30s : %s \n" $var ${!var} ; done
fi 

if [ "run" == "$arg" ]; then
   [ -f "$log" ] && rm $log 
   export T_FOLD 

   $bin
   [ $? -ne 0 ] && echo $BASH_SOURCE run error $bin && exit 1 
fi 

if [ "ana" == "$arg" ]; then
   export T_FOLD 
   ${IPYTHON:-ipython} --pdb -i $(dirname $BASH_SOURCE)/tests/$bin.py 
   [ $? -ne 0 ] && echo $BASH_SOURCE ana error $bin && exit 2 
fi 


exit 0 
