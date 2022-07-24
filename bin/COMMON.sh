#!/bin/bash -l 

usage(){ cat << EOU
COMMON.sh 
============

Edit the common config scripts with com_ 


EOU
}


bindir=$(dirname $BASH_SOURCE)

source $bindir/GEOM_.sh                   # defines and exports : GEOM, GEOMDIR 
source $bindir/OPTICKS_INPUT_PHOTON.sh    # defines and exports : OPTICKS_INPUT_PHOTON, OPTICKS_INPUT_PHOTON_FRAME 


if [ "$GEOM" == "J000" -o "$GEOM" == "J001" -o "$GEOM" == "J002" ]; then 

   case $GEOM in 
     J000) OPTICKS_INPUT_PHOTON_FRAME=NNVT:0:1000 ;;
     J001) OPTICKS_INPUT_PHOTON_FRAME=Hama:0:1000 ;;
     J002) OPTICKS_INPUT_PHOTON_FRAME=NNVT:0:1000 ;;
   esac

   [ -n "$OPTICKS_INPUT_PHOTON_FRAME" ] && export OPTICKS_INPUT_PHOTON_FRAME

   if [ -n "$OPTICKS_INPUT_PHOTON_FRAME" ]; then  
       MOI=$OPTICKS_INPUT_PHOTON_FRAME
       export MOI 
   fi

   export U4VolumeMaker_PVG_WriteNames=1
   export U4VolumeMaker_PVG_WriteNames_Sub=1
fi 

if [ -z "$QUIET" ]; then 
    vars="BASH_SOURCE GEOM OPTICKS_INPUT_PHOTON OPTICKS_INPUT_PHOTON_FRAME MOI"
    for var in $vars ; do printf "%30s : %s\n" $var ${!var} ; done 
fi 

