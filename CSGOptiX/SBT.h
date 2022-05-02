#pragma once

/**
SBT : OptiX 7 RG,MS,HG program data preparation 
=================================================

Aim to minimize geometry specifics in here ...

**/

#include <map>
#include <string>
#include <vector>

#include <optix.h>

#include "Binding.h"
#include "GAS.h"
#include "IAS.h"
#include "sqat4.h"


class Opticks ; 
// TODO: replace use of Opticks here (only used for solid selection) with a less coupled approach.

struct PIP ; 
struct CSGFoundry ; 
struct CSGPrim ; 

struct SBT 
{
    static bool ValidSpec(const char* spec); 
    std::vector<unsigned>  solid_selection ; 
    unsigned long long emm ; 
    const PIP*      pip ; 
    Raygen*       raygen ;
    Miss*         miss ;
    HitGroup*     hitgroup ;
    HitGroup*     check ;

    const CSGFoundry*  foundry ; 
    AS*                top ; 
 
    CUdeviceptr   d_raygen ;
    CUdeviceptr   d_miss ;
    CUdeviceptr   d_hitgroup ;

    OptixShaderBindingTable sbt = {};

    std::map<unsigned, GAS> vgas ; 
    std::vector<IAS> vias ; 

    SBT(const PIP* pip_ ); 


    AS* getAS(const char* spec) const ;
    void setTop(const char* spec) ;
    void setTop(AS* top_) ;
    AS* getTop() const ;

    void init();  
    void createRaygen();  
    void updateRaygen();  

    void createMiss();  
    void updateMiss();  

    void setFoundry(const CSGFoundry* foundry); 

    void createGeom();  
    void createHitgroup();
    void checkHitgroup();

    void createIAS();
    bool isStandardIAS() const ; 
    void createIAS_Standard();
    void createIAS(unsigned ias_idx);
    void createIAS_Selection();
    void createSolidSelectionIAS(unsigned ias_idx, const std::vector<unsigned>& solid_selection);
    void createIAS(const std::vector<qat4>& inst );
    void dumpIAS(const std::vector<qat4>& inst );

    const IAS& getIAS(unsigned ias_idx) const ;

    void createGAS();
    bool isStandardGAS() const ; 
    void createGAS_Standard();
    void createGAS_Selection();
    void createGAS(unsigned gas_idx);
    const GAS& getGAS(unsigned gas_idx) const ;
    std::string descGAS() const ; 

    void setPrimData( HitGroupData& data, const CSGPrim* prim);
    void dumpPrimData( const HitGroupData& data ) const ;
    void checkPrimData( HitGroupData& data, const CSGPrim* prim);

    unsigned getOffset(unsigned shape_idx_ , unsigned layer_idx_ ) const ; 
    unsigned _getOffset(unsigned shape_idx_ , unsigned layer_idx_ ) const ;
    unsigned getTotalRec() const ;

};

