
#include <iostream>
#include <cmath>
#include <cassert>
#include <cstring>


#include "NGLMStream.hpp"

// sysrap-
#include "OpticksCSG.h"

// npy-
#include "NSphere.hpp"
#include "NBBox.hpp"
#include "NPlane.hpp"
#include "NPart.hpp"


float nsphere::radius(){ return param.w ; }
float nsphere::x(){      return param.x ; }
float nsphere::y(){      return param.y ; }
float nsphere::z(){      return param.z ; }

float nsphere::costheta(float z_)
{
   return (z_ - param.z)/param.w ;  
}

// signed distance function

double nsphere::operator()(double px, double py, double pz) 
{
    glm::vec4 p0(px,py,pz,1.0); 
    glm::vec4 p = gtransform ? *gtransform * p0 : p0 ; 

    /*
    if(gtransform)
        std::cout << "nsphere::operator"
                  << " p0 " << p0 
                  << " -> p " << p
                  << " gtransform "  << *gtransform 
                  << std::endl ;  
    */

    float d = glm::distance( glm::vec3(p), center );
    return d - radius_ ;  

} 


nbbox nsphere::bbox()
{
    nbbox bb = make_nbbox();
    bb.min = make_nvec3(param.x - param.w, param.y - param.w, param.z - param.w);
    bb.max = make_nvec3(param.x + param.w, param.y + param.w, param.z + param.w);
    bb.side = bb.max - bb.min ; 

    return bb ; 
}


ndisc nsphere::intersect(nsphere& a, nsphere& b)
{
    // Find Z intersection disc of two Z offset spheres,
    // disc radius is set to zero when no intersection.
    //
    // http://mathworld.wolfram.com/Circle-CircleIntersection.html
    //
    // cf pmt-/dd.py 

    float R = a.radius() ; 
    float r = b.radius() ; 

    // operate in frame of Sphere a 

    float dx = b.x() - a.x() ; 
    float dy = b.y() - a.y() ; 
    float dz = b.z() - a.z() ; 

    assert(dx == 0 && dy == 0 && dz != 0);

    float d = dz ;  
    float dd_m_rr_p_RR = d*d - r*r + R*R  ; 
    float z = dd_m_rr_p_RR/(2.f*d) ;
    float yy = (4.f*d*d*R*R - dd_m_rr_p_RR*dd_m_rr_p_RR)/(4.f*d*d)  ;
    float y = yy > 0 ? sqrt(yy) : 0 ;   


    nplane plane = make_nplane(0,0,1,z + a.param.z) ;
    ndisc  disc = make_ndisc(plane, y) ;

    return disc ;      // return to original frame
}


npart nsphere::part()
{
    float _z = z() ;  
    float r  = radius() ; 

   // hmm this belongs in zsphere not here ???
    nbbox bb = make_nbbox(_z - r, _z + r, -r, r);

    npart p ; 
    p.zero();            
    p.setParam(param) ; 
    p.setTypeCode(CSG_SPHERE); 
    p.setBBox(bb);

    return p ; 
}



npart nsphere::zlhs(const ndisc& dsk)
{
    npart p = part();

    float _z = z() ;  
    float r  = radius() ; 
    nbbox bb = make_nbbox(_z - r, dsk.z(), -dsk.radius, dsk.radius);
    p.setBBox(bb);

    return p ; 
}

npart nsphere::zrhs(const ndisc& dsk)
{
    npart p = part();

    float _z = z() ;  
    float r  = radius() ; 
    nbbox bb = make_nbbox(dsk.z(), _z + r, -dsk.radius, dsk.radius);
    p.setBBox(bb);

    return p ; 
}


