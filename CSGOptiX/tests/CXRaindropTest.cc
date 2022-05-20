/**
CXRaindropTest
======================

**/

#include <cuda_runtime.h>

#include "NP.hh"
#include "SSys.hh"
#include "SCVD.h"
#include "SPath.hh"
#include "SEventConfig.hh"
#include "SOpticks.hh"
#include "SOpticksResource.hh"
#include "SEvent.hh"
#include "OPTICKS_LOG.hh"

#include "SProp.hh"
#include "SSim.hh"

#include "QBnd.hh"
#include "QSim.hh"
#include "QEvent.hh"

#include "CSGFoundry.h"
#include "CSGOptiX.h"

#ifdef WITH_SGLM
#else
#include "Opticks.hh"
#endif

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 
    SCVD::ConfigureVisibleDevices(); 
    SEventConfig::SetRGMode("simulate"); 
#ifdef WITH_SGLM
#else
    Opticks::Configure(argc, argv ); 
#endif
    const char* cfbase = SOpticksResource::CFBase(); 
    const char* outfold = SEventConfig::OutFold();  
    LOG(info) << " outfold [" << outfold << "]"  ; 
    SOpticks::WriteOutputDirScript(outfold) ; // writes CSGOptiXSimulateTest_OUTPUT_DIR.sh in PWD 


    const NP* propcom = SProp::MockupCombination("$IDPath/GScintillatorLib/LS_ori/RINDEX.npy");

    // BASIS GEOMETRY : LOADED IN ORDER TO RAID ITS BOUNDARIES FOR MATERIALS,SURFACES

    CSGFoundry* fd = CSGFoundry::Load(cfbase, "CSGFoundry"); 
    if(fd->hasMeta()) LOG(info) << "fd.meta\n" << fd->meta ; 
    LOG(info) << fd->descComp(); 

    // fabricate some additional boundaries from the basis ones for this simple test 
    std::vector<std::string> specs = { "Rock/perfectAbsorbSurface/perfectAbsorbSurface/Air", "Air///Water" } ;  

    NP* optical_plus = nullptr ; 
    NP* bnd_plus = nullptr ; 
    SSim* ssim = fd->sim ; 
    SSim::Add( &optical_plus, &bnd_plus, ssim->get("optical.npy"), ssim->get("bnd.npy"), specs );
    LOG(info) << std::endl << SSim::DescOptical(optical_plus, bnd_plus)  ; 

    // HMM : THE COMPONENTS ARE MORE RELEVANT TO QSim THAN TO CSGFoundry 
    // PERHAPS SHOULD USE QSim TO MANAGE THEM WITHIN A QSim FOLDER WITHIN CSGFoundry 

    QSim::UploadComponents(ssim->get("icdf.npy"), bnd_plus, optical_plus, propcom ); 



    QSim* sim = new QSim ; 
    std::cout << "sim.desc " << sim->desc() << std::endl ; 

    const char* cfbase_local = SSys::getenvvar("CFBASE_LOCAL") ; assert(cfbase_local) ; 
    LOG(fatal) << "MIXING CSGFoundry combining basis cfbase with cfbase_local "; 
    std::cout << std::setw(20) << "cfbase" << ":" << cfbase << std::endl ; 
    std::cout << std::setw(20) << "cfbase_local" << ":" << cfbase_local  << std::endl ; 

    // load raindrop geometry and customize to use the boundaries added above 
    CSGFoundry* fdl = CSGFoundry::Load(cfbase_local, "CSGFoundry") ; 


    
    // fdl->setOpticalBnd(optical_plus, bnd_plus);    // instanciates bd CSGName using bndplus.names
    // fdl->saveOpticalBnd();                         // DIRTY: persisting new bnd and optical into the source directory : ONLY APPROPRIATE IN SMALL TESTS


    fdl->setPrimBoundary( 0, specs[0].c_str() ); 
    fdl->setPrimBoundary( 1, specs[1].c_str() );   // HMM: notice these boundary changes are not persisted 
    std::cout << "fdl.detailPrim " << std::endl << fdl->detailPrim() ; 
    fdl->upload(); 

    float4 ce = make_float4( 0.f, 0.f, 0.f, 100.f );   // TODO: this should come from the geometry 

    SEventConfig::SetMaxExtent( ce.w );  // must do this config before QEvent::init which happens with CSGOptiX instanciation
    SEventConfig::SetMaxTime( 10.f ); 

    CSGOptiX* cx = CSGOptiX::Create(fdl);  // QSim is QSim::Get is teleported into CSGOptiX 

    cx->setComposition(ce); 
    cx->setGenstep(SEvent::MakeTorchGensteps());     
    cx->simulate();  

    cudaDeviceSynchronize(); 
    cx->event->save(outfold); 

    return 0 ; 
}
