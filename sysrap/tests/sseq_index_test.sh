#!/bin/bash -l 
usage(){ cat << EOU
sseq_index_test.sh
==============

::

    ~/opticks/sysrap/tests/sseq_index_test.sh info

EOU
}

SDIR=$(dirname $(realpath $BASH_SOURCE))
CUDA_PREFIX=${CUDA_PREFIX:-/usr/local/cuda}

source $HOME/.opticks/GEOM/GEOM.sh 
TMP=${TMP:-/tmp/$USER/opticks}

name=sseq_index_test 
export FOLD=$TMP/$name
mkdir -p $FOLD

bin=$FOLD/$name
script=$SDIR/sseq_index_test.py 


executable=G4CXTest 
#executable=CSGOptiXSMTest
export EXECUTABLE=${EXECUTABLE:-$executable}
version=99
export VERSION=${VERSION:-$version}
export BASE=$TMP/GEOM/$GEOM
export LOGDIR=$BASE/$EXECUTABLE/ALL$VERSION
export AFOLD=$LOGDIR/A000 
export BFOLD=$LOGDIR/B000 

vars="BASH_SOURCE SDIR TMP EXECUTABLE BASE GEOM LOGDIR AFOLD BFOLD FOLD"

defarg="info_build_run_ana"
arg=${1:-$defarg}


if [ "${arg/info}" != "$arg" ]; then 
    for var in $vars ; do printf "%30s : %s \n" "$var" "${!var}" ; done 
fi

if [ "${arg/build}" != "$arg" ]; then 
    gcc $SDIR/$name.cc -std=c++11 -lstdc++ -I$SDIR/.. -I$CUDA_PREFIX/include -o $bin 
    [ $? -ne 0 ] && echo $BASH_SOURCE : build error && exit 1 
fi

if [ "${arg/run}" != "$arg" ]; then 
    $bin 
    [ $? -ne 0 ] && echo $BASH_SOURCE : run error && exit 2 
fi 

if [ "${arg/ana}" != "$arg" ]; then 
    ${IPYTHON:-ipython} --pdb  $script 
    [ $? -ne 0 ] && echo $BASH_SOURCE : ana error && exit 3
fi 

exit 0 


