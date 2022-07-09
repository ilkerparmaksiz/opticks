#!/bin/bash -l 
usage(){ cat << EOU
gxr.sh : G4CXRenderTest 
=============================================================================================================

::

    cd ~/opticks/g4cx   # gx
    ./gxr.sh 
    ./gxr.sh run
    ./gxr.sh dbg
    ./gxr.sh grab



name=cx$MOI.jpg
if [ "$(uname)" == "Linux" ]; then
   G4CXRenderTest 
elif [ "$(uname)" == "Darwin" ]; then 
   fold=/tmp/$USER/opticks

   mkdir -p $fold
   cd $fold
   scp P:$fold/$name .

   open $name  

fi 

EOU
}

msg="=== $BASH_SOURCE :"
source ../bin/GEOM_.sh 

case $(uname) in 
  Darwin) defarg="ana"  ;;
  Linux)  defarg="run"  ;;
esac

arg=${1:-$defarg}
bin=G4CXRenderTest
echo $msg arg $arg bin $bin defarg $defarg


eye=-0.4,0,0
moi=-1
export EYE=${EYE:-$eye} 
export MOI=${MOI:-$moi}

loglevels()
{
    export Dummy=INFO
    export SEvt=INFO
    export Ctx=INFO
    #export QSim=INFO
    #export QEvent=INFO 
    export CSGOptiX=INFO
    export G4CXOpticks=INFO 
    #export X4PhysicalVolume=INFO
    #export U4VolumeMaker=INFO
}
loglevels



if [ "${arg/run}" != "$arg" ]; then 
    echo $msg run $bin
    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE run error && exit 1 
fi 

if [ "${arg/dbg}" != "$arg" ]; then 
    case $(uname) in
        Linux) gdb $bin -ex r  ;;
        Darwin) lldb__ $bin ;; 
    esac
    [ $? -ne 0 ] && echo $BASH_SOURCE dbg error && exit 2 
fi


export BASE=/tmp/$USER/opticks/$bin/$GEOM
export FOLD=$BASE 

if [ "${arg/ana}" != "$arg" ]; then 

    export CFBASE=$BASE
    ${IPYTHON:-ipython} --pdb -i tests/$bin.py     
    [ $? -ne 0 ] && echo $BASH_SOURCE ana error && exit 3 
fi 

if [ "${arg/grab}" != "$arg" ]; then 
    source ../bin/rsync.sh $BASE 
fi 

exit 0 

