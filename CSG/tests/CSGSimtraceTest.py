#!/usr/bin/env python
"""
CSG/tests/CSGSimtraceTest.py
===============================

HMM: almost identical to extg4/tests/X4SimtraceTest.py 

TODO: more duplication avoidance but keep distinct mains

"""
import os, logging, builtins, numpy as np
log = logging.getLogger(__name__)
from opticks.ana.fold import Fold 

import matplotlib.pyplot as mp
from opticks.ana.fold import Fold
from opticks.sysrap.sframe import sframe , X, Y, Z
from opticks.CSG.Values import Values 
from opticks.ana.pvplt import mpplt_simtrace_selection_line, mpplt_hist, mpplt_parallel_lines_auto


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    SYMBOLS = os.environ.get("SYMBOLS", None)
    FOLD = os.environ.get("FOLD", None)

    if not FOLD is None:
        s = Fold.Load(symbol="s")
        t = None
        fr = s.sframe
        s_geom = os.environ["GEOM"]

        sv = Values.Find("$FOLD", symbol="sv") if not s_geom is None else None
    elif not SYMBOLS is None:
        ff = Fold.MultiLoad()
        frs = list(filter(None, map(lambda f:f.sframe, ff)))
        assert len(frs) > 0 
        fr = frs[0]       ## HMM: picking first frame, maybe need to form composite bbox from all frames ?
    else:
        assert 0 
    pass

    fig, ax = fr.mp_subplots(mp)  

    if not s is None:
        s_hit = s.simtrace[:,0,3]>0 
        s_pos = s.simtrace[s_hit][:,1,:3]

        ## investigate unexpected top cap intersects : FIXED z2cap TYPO
        ## 
        ##                                                                    intersect pos x > 120.  t > 0 
        ## e = np.logical_and( s.simtrace[:,2,0] > 100., np.logical_and( s.simtrace[:,1,0] > 120. , s.simtrace[:,0,3]>0 )) 
        ## 
        ## e_ori = s.simtrace[e][:100,2,:3]
        ## e_dir = s.simtrace[e][:100,3,:3]
        ## fr.mp_scatter(e_ori, label="e_ori", s=2 ) 
        ## fr.mp_arrow(  e_ori, 10*e_dir, label="e_ori,e_dir", s=2 ) 
    pass

    if s_geom.startswith("nmskSolidMaskVirtual"): 
        r1=sv.get("SolidMask.SolidMaskVirtual.rOuter2.mask_radiu_virtual")
        r2=sv.get("SolidMask.SolidMaskVirtual.rOuter3.mask_radiu_virtual/2")
        z1=sv.get("SolidMask.SolidMaskVirtual.zPlane2.htop_out/2")
        z2=sv.get("SolidMask.SolidMaskVirtual.zPlane3.htop_out+MAGIC_virtual_thickness")
        z0=(z2*r1-z1*r2)/(r1-r2)
        ax.set_ylim( -240, z0+50 )
    pass


    if not s is None and "UNEXPECTED" in os.environ:  

        if s_geom.startswith("nmskSolidMaskVirtual"): 
            w = np.logical_and( np.abs(s.simtrace[:,1,0]) < 220, np.abs(s.simtrace[:,1,2]-98) < 1 ) 
            w_simtrace = s.simtrace[w]
            w_path = "/tmp/simtrace_sample.npy"
            np.save(w_path, w_simtrace)
        else:
            w = np.logical_and( np.abs(s.simtrace[:,1,2]) > 0.20 , s.simtrace[:,0,3]>0 )  
            w_simtrace = s.simtrace[w][::10]
        pass
        log.info("UNEXPECTED w_simtrace : %s " % str(w_simtrace.shape))
        w_ori = w_simtrace[:,2,:3]
        fr.mp_scatter(w_ori, label="w_ori", s=1 )
        mpplt_simtrace_selection_line(ax, w_simtrace, axes=fr.axes, linewidths=2)
    pass

    if not s is None:
        mpplt_parallel_lines_auto( ax, fr.bbox.T, fr.axes, linestyle="dashed" )
    pass

    if not t is None:
        t_hit = t.simtrace[:,0,3]>0 
        t_pos = t.simtrace[t_hit][:,1,:3]
    pass

    if not s is None:
        fr.mp_scatter(s_pos, label="%s" % s_geom, s=1 )
    pass

    if not s is None and hasattr(s,"genstep") and "GS" in os.environ:
        s_gs  = s.genstep[:,5,:3]  
        fr.mp_scatter(s_gs, label="s_gs", s=1 )
    pass


 
    if not s is None and hasattr(s,"simtrace_selection"):
       sts = s.simtrace_selection 
    elif not s is None and "SELECTION" in os.environ:
        #if s_geom 
        #w = np.logical_and( s.simtrace[:,0,3]>0, np.logical_and( s.simtrace[:,1,Z] > -38.9, s.simtrace[:,1,Z] < -20. ))
        w = s.simtrace[:,1,X] > 264.5
        wi = np.where(w)[0]  
        sts = s.simtrace[w][:50]
    else:
        sts = None
    pass  

    if not sts is None:
        mpplt_simtrace_selection_line(ax, sts, axes=fr.axes, linewidths=2)
    pass

    d = getattr(s, "CSGDebug_Cylinder", None) if not s is None else None


    if not t is None:
        fr.mp_scatter(t_pos, label="%s" % t_geom, s=1 )
    pass

    if not "NOLEGEND" in os.environ:
        ax.legend()
    pass
    fig.show()
pass


