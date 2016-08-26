// brap-
#include "BCfg.hh"

#include "NState.hpp"
#include "NLookup.hpp"
#include "NPY.hpp"
#include "NGLM.hpp"
#include "GLMPrint.hpp"
#include "GLMFormat.hpp"

// npy-
#include "Timer.hpp"
#include "TorchStepNPY.hpp"
#include "G4StepNPY.hpp"
#include "Index.hpp"


// numpyserver-
#ifdef WITH_NPYSERVER
#include "numpydelegate.hpp"
#include "numpydelegateCfg.hpp"
#include "numpyserver.hpp"
#endif

// ggeo-
#include "GItemIndex.hh"
#include "GGeo.hh"

// okc-
#include "Bookmarks.hh"
#include "OpticksPhoton.h"
#include "Opticks.hh"
#include "OpticksCfg.hh"
#include "OpticksEvent.hh"
#include "OpticksColors.hh"
#include "Composition.hh"

// opticksgeo-
#include "OpticksHub.hh"
#include "OpticksGeometry.hh"

#include "PLOG.hh"


#define TIMER(s) \
    { \
       if(m_evt)\
       {\
          Timer& t = *(m_evt->getTimer()) ;\
          t((s)) ;\
       }\
       else if(m_opticks) \
       {\
          Timer& t = *(m_opticks->getTimer()) ;\
          t((s)) ;\
       }\
    }


//
// **OpticksHub**
//    Non-viz, hostside intersection of config, geometry and event
//    
//    this means is usable from anywhere, so can mop up config
//

OpticksHub::OpticksHub(Opticks* opticks) 
   :
   m_opticks(opticks),
   m_geometry(NULL),
   m_ggeo(NULL),
   m_composition(NULL),
   m_evt(NULL),
#ifdef WITH_NPYSERVER
   m_delegate(NULL),
   m_server(NULL)
#endif
   m_cfg(NULL),
   m_fcfg(NULL),
   m_state(NULL),
   m_bookmarks(NULL)
{
   init();
}



void OpticksHub::init()
{
    m_composition = new Composition ;   // Composition no longer Viz only 

    m_cfg  = new BCfg("umbrella", false) ; 
    m_fcfg = m_opticks->getCfg();
    add(m_fcfg);

#ifdef WITH_NPYSERVER
    m_delegate    = new numpydelegate ; 
    add(new numpydelegateCfg<numpydelegate>("numpydelegate", m_delegate, false));
#endif

}

bool OpticksHub::hasOpt(const char* name)
{
    return m_fcfg->hasOpt(name);
}
std::string OpticksHub::getCfgString()
{
    return m_cfg->getDescString();
}
OpticksCfg<Opticks>* OpticksHub::getCfg()
{
    return m_fcfg ; 
}
GGeo* OpticksHub::getGGeo()
{
    return m_ggeo ; 
}
NState* OpticksHub::getState()
{
    return m_state ; 
}
OpticksEvent* OpticksHub::getEvent()
{
    return m_evt ; 
}
Opticks* OpticksHub::getOpticks()
{
    return m_opticks ; 
}
Composition* OpticksHub::getComposition()
{
    return m_composition ;  
}
Bookmarks* OpticksHub::getBookmarks()
{
    return m_bookmarks ; 
}
Timer* OpticksHub::getTimer()
{
    return m_evt ? m_evt->getTimer() : m_opticks->getTimer() ; 
}




void OpticksHub::add(BCfg* cfg)
{
    m_cfg->add(cfg); 
}


void OpticksHub::configure()
{
    m_composition->addConfig(m_cfg); 
    //m_cfg->dumpTree();

    int argc    = m_opticks->getArgc();
    char** argv = m_opticks->getArgv();

    LOG(debug) << "OpticksHub::configure " << argv[0] ; 

    m_cfg->commandline(argc, argv);
    m_opticks->configure();      

    if(m_fcfg->hasError())
    {
        LOG(fatal) << "OpticksHub::config parse error " << m_fcfg->getErrorMessage() ; 
        m_fcfg->dump("OpticksHub::config m_fcfg");
        m_opticks->setExit(true);
        return ; 
    }


    bool compute = m_opticks->isCompute();
    bool compute_opt = hasOpt("compute") ;
    if(compute && !compute_opt)
        LOG(warning) << "OpticksHub::configure FORCED COMPUTE MODE : as remote session detected " ;  


    if(hasOpt("idpath")) std::cout << m_opticks->getIdPath() << std::endl ;
    if(hasOpt("help"))   std::cout << m_cfg->getDesc()     << std::endl ;
    if(hasOpt("help|version|idpath"))
    {
        m_opticks->setExit(true);
        return ; 
    }

    if(!m_opticks->isValid())
    {
        // defer death til after getting help
        LOG(fatal) << "OpticksHub::configure OPTICKS INVALID : missing envvar or geometry path ?" ;
        assert(0);
    }

    TIMER("configure");
}


OpticksEvent* OpticksHub::createEvent()
{
    if(!hasOpt("noevent"))
    {
        // TODO: try moving event creation after geometry is loaded, to avoid need to update domains 
        // TODO: organize wrt event loading, currently loading happens latter and trumps this evt ?
        m_evt = m_opticks->makeEvent() ; 
    } 

#ifdef WITH_NPYSERVER
    if(!hasOpt("nonet"))
    {
        m_delegate->liveConnect(m_cfg); // hookup live config via UDP messages
        m_delegate->setEvent(m_evt); // allows delegate to update evt when NPY messages arrive, hmm locking needed ?

        try { 
            m_server = new numpyserver<numpydelegate>(m_delegate); // connect to external messages 
        } 
        catch( const std::exception& e)
        {
            LOG(fatal) << "OpticksHub::config EXCEPTION " << e.what() ; 
            LOG(fatal) << "OpticksHub::config FAILED to instanciate numpyserver : probably another instance is running : check debugger sessions " ;
        }
    }
#endif

    if(m_evt)
    { 
        m_composition->setEvt(m_evt);
        m_composition->setTrackViewPeriod(m_fcfg->getTrackViewPeriod()); 

        bool quietly = true ; 
        NPY<float>* track = m_evt->loadGenstepDerivativeFromFile("track", quietly);
        m_composition->setTrack(track);
    }

    return m_evt ; 
}


void OpticksHub::loadGeometry()
{
    m_geometry = new OpticksGeometry(m_opticks);

    m_geometry->loadGeometry();

    m_ggeo = m_geometry->getGGeo();

    m_ggeo->setComposition(m_composition);

    if(m_evt)
    {    
       // TODO: profit from migrated OpticksEvent 
        LOG(info) << "OpticksGeometry::registerGeometry " << m_opticks->description() ;
        m_evt->setSpaceDomain(m_opticks->getSpaceDomain());
    }    
}



NPY<float>* OpticksHub::loadGenstep()
{
    if(hasOpt("nooptix|noevent")) 
    {
        LOG(warning) << "OpticksHub::loadGenstep skip due to --nooptix/--noevent " ;
        return NULL ;
    }

    unsigned int code = m_opticks->getSourceCode();

    NPY<float>* gs = NULL ; 
    if( code == CERENKOV || code == SCINTILLATION || code == NATURAL )
    {
        gs = loadGenstepFile(); 
    }
    else if(code == TORCH)
    {
        gs = loadGenstepTorch(); 
    }

    TIMER("loadGenstep"); 

    return gs ; 
}



NPY<float>* OpticksHub::loadGenstepFile()
{
    NPY<float>* gs = m_opticks->loadGenstep();
    if(gs == NULL) LOG(fatal) << "OpticksHub::loadGenstepFile FAILED" ;
    assert(gs);

    int modulo = m_fcfg->getModulo();

    //m_parameters->add<std::string>("genstepOriginal",   gs->getDigestString()  );
    //m_parameters->add<int>("Modulo", modulo );

    if(modulo > 0) 
    {    
        LOG(warning) << "OptickHub::loadGenstepFile applying modulo scaledown " << modulo ;
        gs = NPY<float>::make_modulo(gs, modulo);
        //m_parameters->add<std::string>("genstepModulo",   genstep->getDigestString()  );
    }    


    G4StepNPY* g4step = new G4StepNPY(gs);    
    g4step->relabel(CERENKOV, SCINTILLATION); 
    // which code is used depends in the sign of the pre-label 
    // becomes the ghead.i.x used in cu/generate.cu

    if(m_opticks->isDayabay())
    {   
        // within GGeo this depends on GBndLib
        NLookup* lookup = m_ggeo ? m_ggeo->getLookup() : NULL ;
        if(lookup)
        {  
            g4step->setLookup(lookup);   
            g4step->applyLookup(0, 2);  // jj, kk [1st quad, third value] is materialIndex
            //
            // replaces original material indices with material lines
            // for easy access to properties using boundary_lookup GPU side
            //
        }
        else
        {
            LOG(warning) << "OpticksHub::loadGenstepFile not applying lookup" ;
        } 
    }
    return gs ; 
}


NPY<float>* OpticksHub::loadGenstepTorch()
{
    TorchStepNPY* torchstep = m_opticks->makeSimpleTorchStep();

    if(m_ggeo)
    {
        m_ggeo->targetTorchStep(torchstep);
        const char* material = torchstep->getMaterial() ;
        unsigned int matline = m_ggeo->getMaterialLine(material);
        torchstep->setMaterialLine(matline);  

        LOG(debug) << "OpticksHub::loadGenstepTorch"
                   << " config " << torchstep->getConfig() 
                   << " material " << material 
                   << " matline " << matline
                         ;
    }
    else
    {
        LOG(warning) << "OpticksHub::loadGenstepTorch no ggeo, skip setting torchstep material line " ;
    } 

    bool torchdbg = hasOpt("torchdbg");
    torchstep->addStep(torchdbg);  // copyies above configured step settings into the NPY and increments the step index, ready for configuring the next step 

    NPY<float>* gs = torchstep->getNPY();
    if(torchdbg)
    {
        gs->save("$TMP/torchdbg.npy");
    }
    return gs ; 
}


void OpticksHub::targetGenstep()
{
    bool geocenter  = hasOpt("geocenter");
    bool autocam = true ; 
    if(geocenter && m_geometry != NULL )
    {
        glm::vec4 mmce = m_geometry->getCenterExtent();
        m_composition->setCenterExtent( mmce , autocam );
        LOG(info) << "OpticksHub::targetGenstep (geocenter) mmce " << gformat(mmce) ; 
    }
    else if(m_evt)
    {
        glm::vec4 gsce = m_evt->getGenstepCenterExtent();  // need to setGenStepData before this will work 
        m_composition->setCenterExtent( gsce , autocam );
        LOG(info) << "OpticksHub::targetGenstep (!geocenter) gsce " << gformat(gsce) ; 
    }
}



void OpticksHub::loadEvent()
{
    LOG(info) << "OpticksHub::loadEvent START" ;
   
    bool verbose ; 
    m_evt->loadBuffers(verbose=false);

    if(m_evt->isNoLoad())
        LOG(warning) << "OpticksHub::loadEvent LOAD FAILED " ;

    TIMER("loadEvent"); 
}



void OpticksHub::configureViz(NConfigurable* scene)
{
    m_state = m_opticks->getState();
    m_state->setVerbose(false);

    LOG(info) << "OpticksHub::configureViz " << m_state->description();

    m_state->addConfigurable(scene);
    m_composition->addConstituentConfigurables(m_state); // constituents: trackball, view, camera, clipper


    m_bookmarks   = new Bookmarks(m_state->getDir()) ; 
    m_bookmarks->setState(m_state);
    m_bookmarks->setVerbose();
    m_bookmarks->setInterpolatedViewPeriod(m_fcfg->getInterpolatedViewPeriod());

    m_composition->setBookmarks(m_bookmarks);

    m_composition->setOrbitalViewPeriod(m_fcfg->getOrbitalViewPeriod()); 
    m_composition->setAnimatorPeriod(m_fcfg->getAnimatorPeriod()); 

}


void OpticksHub::prepareViz()
{
    glm::uvec4 size = m_opticks->getSize();
    glm::uvec4 position = m_opticks->getPosition() ;

    LOG(info) << "OpticksHub::prepareViz"
              << " size " << gformat(size)
              << " position " << gformat(position)
              ;

    m_composition->setSize( size );
    m_composition->setFramePosition( position );
}


NPY<unsigned char>* OpticksHub::getColorBuffer()
{
    OpticksColors* colors = m_opticks->getColors();

    nuvec4 cd = colors->getCompositeDomain() ; 
    glm::uvec4 cd_(cd.x, cd.y, cd.z, cd.w );
  
    m_composition->setColorDomain(cd_); 

    return colors->getCompositeBuffer() ;
}




void OpticksHub::cleanup()
{
#ifdef WITH_NPYSERVER
    if(m_server) m_server->stop();
#endif
}


OpticksAttrSeq* OpticksHub::getFlagNames()
{
    return m_opticks->getFlagNames();
}
OpticksAttrSeq* OpticksHub::getMaterialNames()
{
    return m_geometry->getMaterialNames();
}
OpticksAttrSeq* OpticksHub::getBoundaryNames()
{
    return m_geometry->getBoundaryNames();
}
std::map<unsigned int, std::string> OpticksHub::getBoundaryNamesMap()
{
    return m_geometry->getBoundaryNamesMap();
}




