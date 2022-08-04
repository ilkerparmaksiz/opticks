/**
G4CXSimulateTest.cc
====================

GGeo creation done when starting from gdml or live G4, still needs Opticks instance,  
TODO: avoid this by replacing with automated SOpticks instanciated by OPTICKS_LOG
and reimplemnting geometry translation in a new more direct "Geo" package  

Formerly did "U4Material::LoadBnd()" here in main 
which created G4 materials from the persisted bnd.npy 
that requires CFBASE or OPTICKS_KEY to derive it
in order to load SSim from $CFBase/CSGFoundry/SSim)

Clearly that is not acceptable when running from live Geant4 or GDML,
so cannot U4Material::LoadBnd from main. 

The new workflow equivalent would be to access the SSim instance
and get the bnd.npy arrays that way.  

**/

#include <cuda_runtime.h>
#include "OPTICKS_LOG.hh"
#include "SEventConfig.hh"
#include "Opticks.hh"   
#include "G4CXOpticks.hh"
#include "U4Sensor.h"
#include "G4PVPlacement.hh"


struct ExampleSensor : public U4Sensor
{
    // In reality would need ctor argument eg junoSD_PMT_v2 to lookup real values 
    unsigned getId(           const G4PVPlacement* pv) const { return pv->GetCopyNo() ; }   
    float getEfficiency(      const G4PVPlacement* pv) const { return 1. ; }
    float getEfficiencyScale( const G4PVPlacement* pv) const { return 1. ; }
}; 


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 
    Opticks::Configure(argc, argv, "--gparts_transform_offset --allownokey" );  

    SEventConfig::SetRGModeSimulate();    
    SEventConfig::SetStandardFullDebug(); // controls which and dimensions of SEvt arrays 

    G4CXOpticks gx ;  
    gx.setGeometry(); 
#ifdef __APPLE__
    return 0 ; 
#endif
    gx.simulate(); 

    cudaDeviceSynchronize(); 
    gx.save();    // $DefaultOutputDir   /tmp/$USER/opticks/$GEOM/SProc::ExecutableName  then ALL from setRelDir
 
    return 0 ; 
}
