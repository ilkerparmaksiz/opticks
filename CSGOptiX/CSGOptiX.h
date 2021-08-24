#pragma once

#include <optix.h>
#include <string>
#include <glm/fwd.hpp>
#include "plog/Severity.h"

struct SMeta ; 
struct NP ; 

struct CSGFoundry ; 
struct CSGView ; 

template <typename T> struct QSim ; 
struct QEvent ; 

struct Params ; 
class Opticks ; 

#if OPTIX_VERSION < 70000
struct Six ; 
#else
struct Ctx ; 
struct PIP ; 
struct SBT ; 
#endif
struct Frame ; 

#include "SRenderer.hh"

struct CSGOptiX : public SRenderer 
{
    static const plog::Severity LEVEL ; 
    static const char* PTXNAME ; 
    static const char* GEO_PTXNAME ; 
    static const char* ENV(const char* key, const char* fallback);

    Opticks*          ok ;  
    int               raygenmode ; 
    bool              flight ; 
    Composition*      composition ; 
 
    const CSGFoundry* foundry ; 
    const char*       prefix ; 
    const char*       outdir ; 
    const char*       cmaketarget ; 
    const char*       ptxpath ; 
    const char*       geoptxpath ; 
    float             tmin_model ; 
    int               jpg_quality ; 

    std::vector<double>  launch_times ;

    Params*           params  ; 
#if OPTIX_VERSION < 70000
    Six* six ;  
#else
    Ctx* ctx ; 
    PIP* pip ; 
    SBT* sbt ; 
    Frame* frame ; 
#endif
    SMeta* meta ; 

    QSim<float>* sim ; 
    QEvent*      evt ;  


    CSGOptiX(Opticks* ok, const CSGFoundry* foundry ); 

    void init(); 
    void initParams();
    void initGeometry();
    void initRender();
    void initSimulate();
 
    void setTop(const char* tspec); 

    // render related 
    void setCEGS(const uint4& cegs_); 
    void setCE(const float4& ce); 
    void setCE(const glm::vec4& ce); 

    void prepareRenderParam(); 
    void prepareSimulateParam(); 
    void prepareParam(); 

    int  render_flightpath(); 
    void saveMeta(const char* jpg_path) const ;
    static std::string Annotation( double dt, const char* bot_line ); 

    // [ fulfil SRenderer protocol base
    double render();    
    void snap(const char* path, const char* bottom_line, const char* top_line=nullptr, unsigned line_height=24); 
    // ]

    void writeFramePhoton(const char* dir, const char* name);

    void setGensteps(const NP* gs);
    double simulate();    
    double launch(unsigned width, unsigned height, unsigned depth) ; 
};

