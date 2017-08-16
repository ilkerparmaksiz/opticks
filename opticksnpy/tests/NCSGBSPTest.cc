#include <iostream>

#include "NSceneConfig.hpp"
#include "NCSG.hpp"
#include "NCSGBSP.hpp"

#include "NPY_LOG.hh"
#include "PLOG.hh"


int main(int argc, char** argv)
{
    PLOG_(argc, argv);
    NPY_LOG__ ;  

    LOG(info) << " argc " << argc << " argv[0] " << argv[0] ;  

    const char* treedir = argc > 1 ? argv[1] : "$TMP/tboolean-hyctrl--/1" ;


    const char* gltfconfig = "csg_bbox_parsurf=1" ;

    const NSceneConfig* config = new NSceneConfig(gltfconfig) ; 

    NCSG* csg = NCSG::LoadTree(treedir, config );

    if(!csg)
    {
        LOG(fatal) << "NO treedir/tree " << treedir ; 
        return 0 ;  
    }

    // NCSGBSP bsp(csg);


    return 0 ; 
}


