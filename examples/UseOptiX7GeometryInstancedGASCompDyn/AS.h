#pragma once

#include <vector>
#include <optix.h>

struct AS
{
    CUdeviceptr             d_buffer;  
    OptixTraversableHandle  handle ; 
    float                   extent0 ; 
    std::vector<float>      extents ; 
    unsigned                num_sbt_rec ; 
};



