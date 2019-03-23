#include "Timer.hpp"

#include "SLog.hh"
#include "OXPPNS.hh"

#include "Opticks.hh"
#include "OpticksEvent.hh"
#include "OpticksCfg.hh"

// okg-
#include "OpticksHub.hh"
#include "GScintillatorLib.hh"

// oxrap-
#include "OContext.hh"
#include "OFunc.hh"
#include "OColors.hh"
#include "OGeo.hh"
#include "OBndLib.hh"
#include "OScintillatorLib.hh"
#include "OSourceLib.hh"
#include "OBuf.hh"
#include "OConfig.hh"

#include "OScene.hh"


#include "PLOG.hh"


#define TIMER(s) \
    { \
       (*m_timer)((s)); \
    }



const plog::Severity OScene::LEVEL = info ; 

OContext* OScene::getOContext()
{
    return m_ocontext ; 
}

OBndLib*  OScene::getOBndLib()
{
    return m_olib ; 
}



/*
//
// dont do this it adds an optix dependency to the interface
// are aiming to get rid of optix from interfaces for 
// easier version hopping
//
optix::Context OScene::getContext()
{
     return m_ocontext->getContext() ; 
}
*/


OScene::OScene(OpticksHub* hub) 
    :   
    m_log(new SLog("OScene::OScene","", debug)),
    m_timer(new Timer("OScene::")),
    m_hub(hub),
    m_ok(hub->getOpticks()),
    m_cfg(m_ok->getCfg()),
    m_ocontext(NULL),
    m_osolve(NULL),
    m_ocolors(NULL),
    m_ogeo(NULL),
    m_olib(NULL),
    m_oscin(NULL),
    m_osrc(NULL),
    m_verbosity(m_ok->getVerbosity()),
    m_use_osolve(false)
{
    init();
    (*m_log)("DONE");
}


/**

OScene::Init
---------------

1. creates OptiX context
2. instanciates the O*Libs which populate the OptiX context 
   from the corresponding libs provided by OpticksHub accessors
   (NB not directly from GGeo or GScene, the Hub mediates)
::

    OColors 
    OSourceLib
    OScintillatorLib
    OGeo
    OBndLib 

**/

void OScene::init()
{
    LOG(LEVEL) << "[" ; 

    m_timer->setVerbose(true);
    m_timer->start();

    std::string builder_   = m_cfg->getBuilder();
    std::string traverser_ = m_cfg->getTraverser();
    const char* builder   = builder_.empty() ? NULL : builder_.c_str() ;
    const char* traverser = traverser_.empty() ? NULL : traverser_.c_str() ;

    LOG(verbose) << "optix::Context::create() START " ; 
    optix::Context context = optix::Context::create();
    LOG(verbose) << "optix::Context::create() DONE " ; 

    m_ocontext = new OContext(context, m_ok);


    // solvers despite being used for geometry intersects have no dependencies
    // as just pure functions : so place them accordingly 
    if(m_use_osolve)
    {  
        m_osolve = new OFunc(m_ocontext, "solve_callable.cu.ptx", "solve_callable", "SolveCubicCallable" ) ; 
        m_osolve->convert();
    }

    LOG(LEVEL) 
          << " ggeobase identifier : " << m_hub->getIdentifier()
          ;

    LOG(debug) << "(OColors)" ;
    m_ocolors = new OColors(context, m_ok->getColors() );
    m_ocolors->convert();

    // formerly did OBndLib here, too soon

    LOG(debug) << "(OSourceLib)" ;
    m_osrc = new OSourceLib(context, m_hub->getSourceLib());
    m_osrc->convert();


    GScintillatorLib* sclib = m_hub->getScintillatorLib() ;
    unsigned num_scin = sclib->getNumScintillators(); 
    const char* slice = "0:1" ;

    LOG(debug) << "(OScintillatorLib)"
               << " num_scin " << num_scin 
               << " slice " << slice  
               ;

    // a placeholder reemission texture is created even when no scintillators
    m_oscin = new OScintillatorLib(context, sclib );
    m_oscin->convert(slice);


    LOG(debug) << "(OGeo)" ;
    m_ogeo = new OGeo(m_ocontext, m_ok, m_hub->getGeoLib(), builder, traverser);
    LOG(debug) << "(OGeo) -> setTop" ;
    m_ogeo->setTop(m_ocontext->getTop());
    LOG(debug) << "(OGeo) -> convert" ;
    m_ogeo->convert();
    LOG(debug) << "(OGeo) done" ;


    LOG(debug) << "(OBndLib)" ;
    m_olib = new OBndLib(context,m_hub->getBndLib());
    m_olib->convert();
    // this creates the BndLib dynamic buffers, which needs to be after OGeo
    // as that may add boundaries when using analytic geometry


    LOG(debug) << m_ogeo->description("OScene::init ogeo");

    LOG(LEVEL) << ")" ;

}


void OScene::cleanup()
{
   if(m_ocontext) m_ocontext->cleanUp();
}


