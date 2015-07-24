
// oglrap-  Frame brings in GL/glew.h GLFW/glfw3.h gleq.h
#include "Frame.hh"
#include "Composition.hh"
#include "FrameCfg.hh"
// these are here just for commandline handling
//  TODO: move commandline handling out of oglrap- into npy-/ggeoview- ? 


// npy-
#include "NPY.hpp"
#include "G4StepNPY.hpp"
#include "NumpyEvt.hpp"
#include "Types.hpp"
#include "Lookup.hpp"
#include "Timer.hpp"
#include "Parameters.hpp"
#include "Times.hpp"
#include "Report.hpp"
#include "stringutil.hpp"

// ggeo-
#include "GLoader.hh"
#include "GCache.hh"
#include "GMergedMesh.hh" 
class GBoundaryLib ; 



// assimpwrap-
#include "AssimpGGeo.hh"

// optixrap-
#include "OptiXEngine.hh"
#include "RayTraceConfig.hh"


#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include "boost/log/utility/setup.hpp"
#define LOG BOOST_LOG_TRIVIAL
// trace/debug/info/warning/error/fatal


void logging_init()
{
   // see blogg-
    boost::log::core::get()->set_filter
    (
        boost::log::trivial::severity >= boost::log::trivial::info
    );

    boost::log::add_console_log(
        std::cerr, 
        boost::log::keywords::format = "[%TimeStamp%]: %Message%",
        boost::log::keywords::auto_flush = true
    );  

    boost::log::add_common_attributes();
}


int main(int argc, char** argv)
{
    Parameters p ; 
    Timer t ; 
    t.setVerbose(true);
    t.start();

    logging_init();

    LOG(info) << argv[0] ; 

    GCache cache("GGEOVIEW_") ; 
    const char* idpath = cache.getIdPath();
    const char* det = cache.getDetector();

    Types types ;  
    types.readFlags("$ENV_HOME/graphics/ggeoview/cu/photon.h");

    Frame frame ;
    Cfg cfg("umbrella", false) ; // collect other Cfg objects
    FrameCfg<Frame>* fcfg = new FrameCfg<Frame>("frame", &frame,false);
    cfg.add(fcfg);

    cfg.commandline(argc, argv);
    t.setCommandLine(cfg.getCommandLine()); 

    if(fcfg->hasOpt("idpath")) std::cout << idpath << std::endl ;
    if(fcfg->hasOpt("help"))   std::cout << cfg.getDesc() << std::endl ;
    if(fcfg->isAbort()) exit(EXIT_SUCCESS); 


    unsigned int numcol = 64 ; 
    bool nogeocache = fcfg->hasOpt("nogeocache");
    bool nooptix    = fcfg->hasOpt("nooptix");
    bool compute    = true ; 
    assert(nogeocache == false) ;
    assert(nooptix == false);

    GLoader loader ;
    loader.setTypes(&types);
    loader.setCache(&cache);
    loader.setImp(&AssimpGGeo::load);    // setting GLoaderImpFunctionPtr
    loader.load(nogeocache, fcfg->getRepeatIndex());

    GMergedMesh* mm = loader.getMergedMesh(); 
    GBoundaryLib* blib = loader.getBoundaryLib();
    Lookup* lookup = loader.getMaterialLookup();

    unsigned int target = 0 ; 
    gfloat4 ce = mm->getCenterExtent(target);

    LOG(info) << "main drawable ce: " 
              << " x " << ce.x
              << " y " << ce.y
              << " z " << ce.z
              << " w " << ce.w
              ;

    Composition composition ;   
    composition.setDomainCenterExtent(ce);     // index 0 corresponds to entire geometry
    composition.setTimeDomain( gfloat4(0.f, fcfg->getTimeMax(), 0.f, 0.f) );  
    composition.setColorDomain( gfloat4(0.f, numcol, 0.f, 0.f));


    const char* typ = "cerenkov" ; 
    const char* tag = "1" ;  
    NPY<float>* npy = NPY<float>::load(typ, tag, det ) ;
    npy->Summary();

    G4StepNPY genstep(npy);    
    genstep.setLookup(lookup); 
   
    if(cache.isJuno())
    {
        LOG(warning) << "main: kludge skip genstep.applyLookup for JUNO " ;
    }
    else
    {   
        genstep.applyLookup(0, 2);   // translate materialIndex (1st quad, 3rd number) from chroma to GGeo 
    }


    NumpyEvt evt ;
    evt.setMaxRec(fcfg->getRecordMax());  // must set this before setGenStepData to have effect
    evt.setGenstepData(npy, nooptix); 


    OptiXEngine engine("GGeoView", OptiXEngine::COMPUTE ) ;       
    engine.setFilename(idpath);
    engine.setMergedMesh(mm);   
    engine.setBoundaryLib(blib);   
    engine.setNumpyEvt(&evt);
    engine.setComposition(&composition);                 
    engine.setEnabled(!nooptix);
    engine.setBounceMax(fcfg->getBounceMax());  // 0:prevents any propagation leaving generated photons
    engine.setRecordMax(evt.getMaxRec());       // 1:to minimize without breaking machinery 


    int rng_max = getenvint("CUDAWRAP_RNG_MAX",-1);
    assert(rng_max >= 1e6); 

    engine.setRngMax(rng_max);


    p.add<std::string>("Type", typ );
    p.add<std::string>("Tag", tag );
    p.add<std::string>("Detector", det );
    p.add<unsigned int>("NumGensteps", evt.getNumGensteps());
    p.add<unsigned int>("RngMax",     engine.getRngMax() );
    p.add<unsigned int>("NumPhotons", evt.getNumPhotons());
    p.add<unsigned int>("NumRecords", evt.getNumRecords());
    p.add<unsigned int>("BounceMax", engine.getBounceMax() );
    p.add<unsigned int>("RecordMax", engine.getRecordMax() );


    LOG(info)<< " ******************* main.OptiXEngine::init creating OptiX context, when enabled *********************** " ;
    engine.init();  
    t("initOptiX"); 
    LOG(info) << "main.OptiXEngine::init DONE "; 

   
    LOG(info)<< " ******************* (main) OptiXEngine::generate + propagate  *********************** " ;
    engine.generate();     
    t("generatePropagate"); 

    LOG(info) << "main.OptiXEngine::generate DONE "; 

    engine.downloadEvt();

    t("downloadEvt"); 


    NPY<float>* dpho = evt.getPhotonData();
    dpho->setVerbose();
    dpho->save("ox%s", typ,  tag, det);

    NPY<short>* drec = evt.getRecordData();
    drec->setVerbose();
    drec->save("rx%s", typ,  tag, det);

    NPY<unsigned long long>* dhis = evt.getSequenceData();
    dhis->setVerbose();
    dhis->save("ph%s", typ,  tag, det);



    t("evtSave"); 


    t.stop();

    p.dump();
    t.dump();

    Report r ; 
    r.add(p.getLines()); 
    r.add(t.getLines()); 

    Times* ts = t.getTimes();
    ts->save("$IDPATH/times", Times::name(typ, tag).c_str());

    char rdir[128];
    snprintf(rdir, 128, "$IDPATH/report/%s/%s", tag, typ ); 
    r.save(rdir, Report::name(typ, tag).c_str());  // with timestamp prefix


}


