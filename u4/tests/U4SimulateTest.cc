/**
U4SimulateTest.cc
==================

All the Geant4 setup happens in U4App::Create from U4App.h

TODO: incorporate J_PMTSIM_LOG_ hookup into OPTICKS_LOG

**/

#include "U4App.h"    
#include "SEvt.hh"
#include "OPTICKS_LOG.hh"

#ifdef WITH_PMTSIM
#include "J_PMTSIM_LOG.hh"
#endif

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 
#ifdef WITH_PMTSIM
    J_PMTSIM_LOG_(0); 
#endif


    LOG(info) << SLOG::Banner() ; 

    U4App* app = U4App::Create() ;  
    app->BeamOn(); 
    delete app ; 

    LOG(info) << SLOG::Banner() << " " << " savedir " << SEvt::GetSaveDir() ; 
    return 0 ; 
}

