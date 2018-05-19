// OpticksCMakeConfig.hh header is generated at configure time
// based on parsing of OptiX and Geant4 headers if they are 
// found.  See oxrap/CMakeLists.txt cfg4/CMakeLists.txt CMakeLists.txt
//
#include "OpticksCMakeConfig.hh"
#include "PLOG.hh"

int main(int argc, char** argv)
{
    PLOG_(argc, argv);


#if OXRAP_OPTIX_VERSION >= 3080
    LOG(info) << " OXRAP_OPTIX_VERSION >= 3080 : " << OXRAP_OPTIX_VERSION  ;
#else
    LOG(info) << " (NOT) OXRAP_OPTIX_VERSION >= 3080 : " << OXRAP_OPTIX_VERSION  ;
#endif


#if OXRAP_OPTIX_VERSION == 3080
    LOG(info) << " OXRAP_OPTIX_VERSION == 3080 : " << OXRAP_OPTIX_VERSION  ;
#elif OXRAP_OPTIX_VERSION == 3090
    LOG(info) << " OXRAP_OPTIX_VERSION == 3090 : " << OXRAP_OPTIX_VERSION  ;
#elif OXRAP_OPTIX_VERSION == 40000
    LOG(info) << " OXRAP_OPTIX_VERSION == 40000 : " << OXRAP_OPTIX_VERSION  ;
#elif OXRAP_OPTIX_VERSION == 400000
    LOG(info) << " OXRAP_OPTIX_VERSION == 400000 : " << OXRAP_OPTIX_VERSION  ;
#elif OXRAP_OPTIX_VERSION == 50001
    LOG(info) << " OXRAP_OPTIX_VERSION == 50001 : " << OXRAP_OPTIX_VERSION  ;
#else
    LOG(info) << " (NOT) OXRAP_OPTIX_VERSION == 3080,3090,4000,400000,50001 : " << OXRAP_OPTIX_VERSION  ;
#endif




#if OXRAP_OPTIX_VERSION == 3080 || OXRAP_OPTIX_VERSION == 3090 
    LOG(info) << " OXRAP_OPTIX_VERSION == 3080 || OXRAP_OPTIX_VERSION == 3090 : " << OXRAP_OPTIX_VERSION  ;
#elif OXRAP_OPTIX_VERSION == 40000
    LOG(info) << " OXRAP_OPTIX_VERSION == 40000 : " << OXRAP_OPTIX_VERSION  ;
#elif OXRAP_OPTIX_VERSION == 400000
    LOG(info) << " OXRAP_OPTIX_VERSION == 400000 : " << OXRAP_OPTIX_VERSION  ;
#elif OXRAP_OPTIX_VERSION == 50001
    LOG(info) << " OXRAP_OPTIX_VERSION == 50001 : " << OXRAP_OPTIX_VERSION  ;
#else
    LOG(info) << " (NOT) OXRAP_OPTIX_VERSION == 3080,3090,4000,400000,50001 : " << OXRAP_OPTIX_VERSION  ;
#endif





    return 0 ; 
}
