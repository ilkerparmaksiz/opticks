#!/usr/bin/env python
"""
scan.py
============

scan.py is sibling to bench.py : aiming for a cleaner 
and more general approach to metadata presentation 
based on meta.py (rather than the old metadata.py)


"""
import os, re, logging, sys, argparse
from collections import OrderedDict as odict
import numpy as np
log = logging.getLogger(__name__)

from opticks.ana.datedfolder import DatedFolder
from opticks.ana.meta import Meta


if __name__ == '__main__':
    base = sys.argv[1] if len(sys.argv) > 1 else "." 
    dirs, dfolds, dtimes = DatedFolder.find(base)
    assert len(dfolds) == len(dtimes)
    print( "dirs : %d  dtimes : %d " % (len(dirs), len(dtimes) ))


    ## arrange into groups of runs with the same runstamp/datedfolder
    assert len(dfolds) == len(dtimes) 
    order = sorted(range(len(dfolds)), key=lambda i:dtimes[i])   ## sorted by run datetimes

    dump_ = lambda m,top:"\n".join(map(lambda kv:"  %30s : %s " % (kv[0],kv[1]),m.d[top].items() )) 

    photons_ = lambda m:m["parameters.NumPhotons"]

    q=odict()
    q["ok1"] = lambda m:m["OpticksEvent_launch.launch001"]
    q["ok2"] = lambda m:m["DeltaTime.OPropagator::launch_0"]
    q["g4"]  = lambda m:m["DeltaTime.CG4::propagate_0"]
        
    for i in order:
        df = dfolds[i] 
        dt = dtimes[i] 

        udirs = filter(lambda _:_.endswith(df),dirs)
        #print("\n".join(udirs))
        if len(udirs) == 0: continue

        mm = [Meta(p, base) for p in udirs]
        assert len(mm) == 2  

        tag0 = int(mm[0].parentfold)  
        tag1 = int(mm[1].parentfold)  

        ok = 0 if tag0 > 0 else 1 
        g4 = 1 if tag1 < 0 else 0
        assert ok ^ g4, ( ok, g4, tag0, tag1 )

        #print(mm[ok])
        #print(mm[g4])

        nn = list(set(map(photons_, mm)))
        assert len(nn) == 1
        n = nn[0]

        vq = odict()
        for k,v in q.items():
            vq[k] = float(v(mm[ok if k.startswith("ok") else g4]))
        pass

        vq["g4/ok1"] = vq["g4"]/vq["ok1"]
        vq["g4/ok2"] = vq["g4"]/vq["ok2"]

        print(" tag0:%-3d tag1:%-3d  n:%-7d     %s     " % (tag0, tag1, n,  " ok1:%(ok1)10.4f  ok2:%(ok2)10.4f  g4:%(g4)10.4f   g4/ok1:%(g4/ok1)10.1f  g4/ok2:%(g4/ok2)10.1f    " % vq  )  )

        #print(dump_(mm[0],"parameters"))
        #print(dump_(mm[1],"OpticksEvent_launch"))

        





