#pragma once

#include "OpticksCSG.h"
#include "NPart.h"
#include "NQuad.hpp"

struct npart ; 
struct nbbox ; 

struct NPY_API npart 
{
    nquad q0 ;  // x,y,z,w (float): param 
    nquad q1 ;  // x,y,z,w (uint) -/index/boundary/flags
    nquad q2 ;  // x,y,z (float):bbmin   w(uint):typecode  
    nquad q3 ;  // x,y,z (float):bbmax

    void zero();
    void dump(const char* msg);
    void setTypeCode(OpticksCSG_t typecode);
    void setBBox(const nbbox& bb);
    void setParam(const nvec4& param);
    void setParam(float x, float y, float z, float w);

    OpticksCSG_t getTypeCode();
    bool isPrimitive();

    //NB same memory used for different purposes for CSG operator nodes
    //   distinguish usage typecode or isPrimitive

    void setLeft(unsigned left);
    void setRight(unsigned right);
    unsigned getLeft();
    unsigned getRight();

    static void traverse( npart* tree, unsigned numNodes, unsigned node );

};



