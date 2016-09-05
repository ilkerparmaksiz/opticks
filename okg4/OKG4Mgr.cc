#include "OKG4Mgr.hh"

class NConfigurable ; 

#include "Timer.hpp"

#include "Opticks.hh"       // okc-
#include "OpticksEvent.hh"
#include "OpticksHub.hh"    // opticksgeo-
#include "OpticksIdx.hh"    // opticksgeo-

#ifdef WITH_OPTIX
#include "OpViz.hh"     // optixgl-
#include "OpEngine.hh"  // opticksop-
#endif

#define GUI_ 1
#include "OpticksViz.hh"

#include "CG4.hh"
#include "CCollector.hh"

#include "PLOG.hh"
#include "OKG4_BODY.hh"

#define TIMER(s) \
    { \
       if(m_hub)\
       {\
          Timer& t = *(m_hub->getTimer()) ;\
          t((s)) ;\
       }\
    }

OKG4Mgr::OKG4Mgr(int argc, char** argv) 
    :
    m_ok(new Opticks(argc, argv, true)),  // true: integrated running 
    m_hub(new OpticksHub(m_ok, true)),    // true: configure, loadGeometry and setupInputGensteps immediately
    m_idx(new OpticksIdx(m_hub)),
    m_collector(new CCollector(m_hub->getLookup())),   // after CG4 loads geometry, for material code cross-referenceing in NLookup
    m_g4(new CG4(m_hub, true)),   // true: configure and initialize immediately 
    m_viz(m_ok->isCompute() ? NULL : new OpticksViz(m_hub, m_idx)),
#ifdef WITH_OPTIX
    m_ope(new OpEngine(m_hub)),
    m_opv(m_viz ? new OpViz(m_ope,m_viz) : NULL),
#endif
    m_placeholder(0)
{
    m_collector->postinit();   // closes Lookup only after CG4 has been able to override LookupA

    init();

    LOG(fatal) << "OKG4Mgr::OKG4Mgr DONE" ;  
}

void OKG4Mgr::init()
{
    LOG(fatal) << "OKG4Mgr::init" ;  

    if(m_viz) 
    {
        m_hub->configureState(m_viz->getSceneConfigurable()) ;    // loads/creates Bookmarks

        m_viz->prepareScene();      // setup OpenGL shaders and creates OpenGL context (the window)
 
        m_viz->uploadGeometry();    // Scene::uploadGeometry, hands geometry to the Renderer instances for upload
    }

#ifdef WITH_OPTIX
    m_ope->prepareOptiX();                // creates OptiX context and populates with geometry by OGeo, OScintillatorLib, ... convert methods 

    if(m_opv) m_opv->prepareTracer();     // creates ORenderer, OTracer
#endif

    LOG(fatal) << "OKG4Mgr::init DONE" ;
}





void OKG4Mgr::propagate()
{
    LOG(fatal) << "OKG4Mgr::propagate" ;

    unsigned code = m_ok->getSourceCode();

    m_g4->propagate();

    NPY<float>* gs = code == G4GUN ? m_collector->getGensteps() : m_hub->getGensteps() ;  i
     // collected from G4  OR input gensteps

    int n_gs  = gs ? gs->getNumItems() : -1 ; 

    LOG(fatal) << "OKG4Mgr::propagate n_gs " << n_gs ;  

    if( n_gs <= 0 )
    {
          LOG(fatal) << "OKG4Mgr::propagate"
                     << " SKIPPING as no collected optical gensteps (ie Cerenkov or scintillation gensteps) "
                     << " or fabricated torch gensteps  "
                     ;
         return ;  
    }

    m_hub->initOKEvent(gs);           // make a new evt 

    if(m_viz)
    { 
        // handling target option inside Scene is inconvenient  TODO: centralize
        int target = m_viz->getTarget();  // hmm target could be non oglrap- specific 
        if(target == 0) 
            m_hub->target();           // if not Scene targetted, point Camera at gensteps of last created evt

        m_viz->uploadEvent();        // allocates GPU buffers with OpenGL glBufferData
    }

#ifdef WITH_OPTIX
    m_ope->preparePropagator();              // creates OptiX buffers and OBuf wrappers as members of OPropagator

    m_ope->seedPhotonsFromGensteps();        // distributes genstep indices into the photons buffer

    if(m_ok->hasOpt("onlyseed")) exit(EXIT_SUCCESS);

    m_ope->initRecords();                    // zero records buffer, not working in OptiX 4 in interop 

    if(!m_ok->hasOpt("nooptix|noevent|nopropagate"))
    {
        m_ope->propagate();                  // perform OptiX GPU propagation 
    }



    indexPropagation();


    if(m_ok->hasOpt("save"))
    {
        if(m_viz) m_viz->downloadEvent();
        m_ope->downloadEvt();

        m_idx->indexEvtOld();  // hostside checks, when saving makes sense
        m_idx->indexSeqHost();


        m_hub->save();
    }
#endif
    LOG(fatal) << "OKG4Mgr::propagate DONE" ;
}




void OKG4Mgr::indexPropagation()
{
    OpticksEvent* evt = m_hub->getOKEvent();
    assert(evt->isOK()); // is this always so ?
    if(!evt->isIndexed())
    {
#ifdef WITH_OPTIX 
        m_ope->indexSequence();
#endif
        m_idx->indexBoundariesHost();
    }
    if(m_viz) m_viz->indexPresentationPrep();
}

void OKG4Mgr::visualize()
{
    if(!m_viz) return ; 
    m_viz->prepareGUI();
    m_viz->renderLoop();    
}

void OKG4Mgr::cleanup()
{
#ifdef WITH_OPTIX
    if(m_ope) m_ope->cleanup();
#endif
    m_hub->cleanup();
    if(m_viz) m_viz->cleanup();
    m_ok->cleanup(); 
    m_g4->cleanup(); 
}

