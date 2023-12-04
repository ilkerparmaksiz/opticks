#!/bin/bash -l 
usage(){ cat << EOU
sreport.sh : summarize SEvt metadata into eg ALL0_sreport FOLD 
================================================================

The output NPFold and summary NPFold from several scripts are managed 
by this sreport.sh script. 

+-----------------------------------------+---------------+
|  bash script that creates SEvt NPFold   |   JOB tags    |      
+=========================================+===============+
| ~/opticks/CSGOptiX/cxs_min.sh           |  N3           |
+-----------------------------------------+---------------+
| ~/opticks/g4cx/tests/G4CXTest_GEOM.sh   |  N2,N4        |
+-----------------------------------------+---------------+
| ~/j/okjob.sh + ~/j/jok.bash             |  L1,N1        |
+-----------------------------------------+---------------+

::

   ~/opticks/sysrap/tests/sreport.sh

   JOB=N3 ~/opticks/sysrap/tests/sreport.sh           ## summarize SEvt folders
   JOB=N3 ~/opticks/sysrap/tests/sreport.sh grab      ## from remove to local 
   JOB=N3 ~/opticks/sysrap/tests/sreport.sh ana       ## local plotting 

   runprof=1 ~/opticks/sysrap/tests/sreport.sh ana   


**JOB**
   selects the output and summary folders of various scripts

**build**
   standalone build of the sreport binary, CAUTION the binary is also built 
   and installed by the standard "om" build 

**run**  
   sreport loads the SEvt subfolders "p001" "n001" etc beneath 
   the invoking directory in NoData(metadata only) mode and 
   writes a summary NPFold into FOLD directory 
   
   NB DEV=1 uses the standalone binary built by this script, and 
   not defining DEV uses that standardly intalled sreport binary 

**grab**
   grab command just rsyncs the summary FOLD back to laptop for 
   metadata plotting without needing to transfer the potentially 
   large SEvt folders. 

**ana**
   python plotting using ~/opticks/sysrap/tests/sreport.py::

       substamp=1 ~/opticks/sysrap/tests/sreport.sh ana 
       subprofile=1 ~/opticks/sysrap/tests/sreport.sh ana 



Note that *sreport* executable can be used without this script 
by invoking it from appropriate directories, examples are shown below.

Invoking Directory 
   /data/blyth/opticks/GEOM/J23_1_0_rc3_ok0/CSGOptiXSMTest/ALL0
Summary "FOLD" Directory 
   /data/blyth/opticks/GEOM/J23_1_0_rc3_ok0/CSGOptiXSMTest/ALL0_sreport

EOU
}

SOURCE=$([ -L $BASH_SOURCE ] && readlink $BASH_SOURCE || echo $BASH_SOURCE)
SDIR=$(cd $(dirname $SOURCE) && pwd)
name=sreport
src=$SDIR/$name.cc
script=$SDIR/$name.py

DEV=1
if [ -z "$DEV" ]; then
    bin=$name                                   ## standard binary 
    defarg="run_info_ana"
else
    bin=${TMP:-/tmp/$USER/opticks}/$name/$name    ## standalone binary
    #defarg="build_run_info_ana"
    defarg="build_run_info_noa"
fi
arg=${1:-$defarg}


if [ "$bin" == "$name" ]; then
    echo $BASH_SOURCE : using standard binary 
else
    mkdir -p $(dirname $bin)
fi

source $HOME/.opticks/GEOM/GEOM.sh 

job=N5
JOB=${JOB:-$job}

DIR=unknown 
case $JOB in 
  L1) DIR=/hpcfs/juno/junogpu/blyth/tmp/GEOM/$GEOM/jok-tds/ALL0 ;;
  N1) DIR=/data/$USER/opticks/GEOM/$GEOM/jok-tds/ALL0 ;;
  N2) DIR=/data/$USER/opticks/GEOM/$GEOM/G4CXTest/ALL0 ;;
  N3) DIR=/data/$USER/opticks/GEOM/$GEOM/CSGOptiXSMTest/ALL2 ;;
  N4) DIR=/data/$USER/opticks/GEOM/$GEOM/G4CXTest/ALL2 ;;
  N5) DIR=/data/$USER/opticks/GEOM/$GEOM/G4CXTest/ALL3 ;;
esac

export FOLD=${DIR}_${name}   ## FOLD is output directory used by binary, export it for python 
export MODE=2                ## 2:matplotlib plotting 

vars="0 BASH_SOURCE SDIR JOB DIR FOLD MODE name bin script"

if [ "${arg/info}" != "$arg" ]; then
    for var in $vars ; do printf "%25s : %s \n" "$var" "${!var}" ; done 
fi 

if [ "${arg/build}" != "$arg" ]; then 
    gcc $src -g -std=c++11 -lstdc++ -I$SDIR/.. -o $bin 
    [ $? -ne 0 ] && echo $BASH_SOURCE : build error && exit 1
fi

if [ "${arg/dbg}" != "$arg" ]; then 
    cd $DIR
    [ $? -ne 0 ] && echo $BASH_SOURCE : NO SUCH DIRECTORY : JOB  $JOB DIR $DIR && exit 0 

    dbg__ $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE : dbg error && exit 3
fi

if [ "${arg/run}" != "$arg" ]; then 
    cd $DIR
    [ $? -ne 0 ] && echo $BASH_SOURCE : NO SUCH DIRECTORY : JOB  $JOB DIR $DIR && exit 0 

    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE : run error && exit 3
fi

if [ "${arg/grab}" != "$arg" ]; then 
    echo $BASH_SOURCE : grab FOLD $FOLD 
    source $OPTICKS_HOME/bin/rsync.sh $FOLD
    [ $? -ne 0 ] && echo $BASH_SOURCE : grab error && exit 4
fi

if [ "${arg/noa}" != "$arg" ]; then 
    echo $BASH_SOURCE : noa : no analysis exit 
    exit 0
fi

if [ "${arg/ana}" != "$arg" ]; then 
    ${IPYTHON:-ipython} --pdb -i $script
    [ $? -ne 0 ] && echo $BASH_SOURCE : ana error && exit 3
fi

if [ "${arg/info}" != "$arg" ]; then
    for var in $vars ; do printf "%25s : %s \n" "$var" "${!var}" ; done 
fi 

exit 0 

