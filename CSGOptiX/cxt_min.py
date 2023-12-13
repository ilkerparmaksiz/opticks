#!/usr/bin/env python
"""
cxt_min.py 
===========

TODO: cherry pick from tests/CSGOptiXSimtraceTest.py and simplify 
for minimal simtrace plotting 

"""
import os, numpy as np
from opticks.ana.fold import Fold
from opticks.sysrap.sevt import SEvt

GLOBAL = int(os.environ.get("GLOBAL","0")) == 1
MODE = int(os.environ.get("MODE","3")) 

if MODE in [2,3]:
    from opticks.ana.pvplt import *   
    # HMM this import overrides MODE, so need to keep defaults the same 
pass

if __name__ == '__main__':

    print("GLOBAL:%d MODE:%d" % (GLOBAL,MODE))

    e = SEvt.Load(symbol="e")
    print(repr(e))
    label = e.f.base

    sf = e.f.sframe
    gs = e.f.genstep
    st = e.f.simtrace
    w2m = sf.w2m
 
    gs_pos = gs[:,1]
    all_one = np.all( gs_pos[:,3] == 1. )
    all_zero = np.all( gs_pos[:,:3]  == 0 )
    assert all_one  # SHOULD ALWAYS BE 1. 
    assert all_zero # NOT ALWAYS I GUESS ? 
    gs_tra = gs[:,2:]
    assert gs_tra.shape[1:] == (4,4) 


    ggrid = np.zeros( (len(gs), 4 ), dtype=np.float32 )    
    for i in range(len(ggrid)): ggrid[i] = np.dot(gs_pos[i], gs_tra[i])
    ## np.matmul/np.tensordot/np.einsum can probably do this without the loop 
    lgrid = np.dot( ggrid, w2m )
    ugrid = ggrid if GLOBAL else lgrid


    gpos = st[:,1].copy() 
    gpos[...,3] = 1.
    lpos = np.dot( gpos, w2m )
    upos = gpos if GLOBAL else lpos



    H,V = 0,2 
  
    if MODE == 2:
        pl = mpplt_plotter(label=label)
        fig, axs = pl
        ax = axs[0]
        ax.scatter( upos[:,H], upos[:,V], s=0.1 )
        ax.scatter( ugrid[:,H], ugrid[:,V], s=0.1, color="r" )
        fig.show()
    elif MODE == 3:
        pl = pvplt_plotter(label)
        pvplt_viewpoint(pl)   # sensitive to EYE, LOOK, UP envvars
        pvplt_frame(pl, sf, local=not GLOBAL )
        pl.add_points(upos[:,:3])
        pl.add_points(ugrid[:,:3], color="r" ) 
        pl.show()
    else:
        pass
    pass
pass


 

