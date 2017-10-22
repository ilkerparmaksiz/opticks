
#include <iostream>
#include <cmath>
#include <cassert>
#include <cstring>

#include "NGLMExt.hpp"

// sysrap-
#include "OpticksCSG.h"

// npy-
#include "NCylinder.hpp"
#include "NBBox.hpp"
#include "NPlane.hpp"
#include "NPart.hpp"
#include "Nuv.hpp"
#include "NCone.hpp"

#include "PLOG.hh"

#include "NCylinder.h"




NPY_API void init_cylinder(ncylinder& n, const nquad& param, const nquad& param1 )
{
    n.param = param ; 
    n.param1 = param1 ;

    bool z_ascending = n.z2() > n.z1() ;
    if(!z_ascending) n.pdump("init_cylinder z_ascending FAIL ");
    assert( z_ascending  );
}


nbbox ncylinder::bbox() const 
{
    float r = radius();
    glm::vec3 c = center();

    glm::vec3 mx(c.x + r, c.y + r, z2() );
    glm::vec3 mi(c.x - r, c.y - r, z1() );

    nbbox bb = make_bbox(mi, mx, complement);

    return gtransform ? bb.make_transformed(gtransform->t) : bb ; 
}

float ncylinder::operator()(float x_, float y_, float z_) const 
{
    glm::vec4 p(x_,y_,z_,1.0); 
    if(gtransform) p = gtransform->v * p ; 

    float dinf = glm::distance( glm::vec2(p.x, p.y), glm::vec2(x(), y()) ) - radius() ;  // <- no z-dep

    float qcap_z = z2() ;  // typically +ve   z2>z1  
    float pcap_z = z1() ;  // typically -ve

    float d_PQCAP = fmaxf( p.z - qcap_z, -(p.z - pcap_z) );

    float sd = fmaxf( d_PQCAP, dinf );

/*
    std::cout 
          << "ncylinder" 
          << " p " << p 
          << " dinf " << dinf
          << " dcap " << dcap
          << " sd " << sd
          << std::endl 
          ;
*/
    return complement ? -sd : sd ; 
} 

glm::vec3 ncylinder::gseedcenter() const 
{
    return gtransform == NULL ? center() : glm::vec3( gtransform->t * glm::vec4(center(), 1.f ) ) ;
}

glm::vec3 ncylinder::gseeddir() const 
{
    glm::vec4 dir(1,0,0,0);   // Z: not a good choice as without endcap fail to hit 
    if(gtransform) dir = gtransform->t * dir ; 
    return glm::vec3(dir) ;
}

void ncylinder::pdump(const char* msg ) const 
{
    std::cout 
              << std::setw(10) << msg 
              << " label " << ( label ? label : "no-label" )
              << " center " << center() 
              << " radius " << radius() 
              << " z1 " << z1()
              << " z2 " << z2()
              << " gseedcenter " << gseedcenter()
              << " gtransform " << !!gtransform 
              << std::endl ; 

    if(verbosity > 1 && gtransform) std::cout << *gtransform << std::endl ;
}







unsigned ncylinder::par_nsurf() const 
{
   return 3 ; 
}
int ncylinder::par_euler() const 
{
   return 2 ; 
}
unsigned ncylinder::par_nvertices(unsigned /*nu*/, unsigned /*nv*/) const 
{
   return 0 ; 
}

glm::vec3 ncylinder::par_pos_model(const nuv& uv) const 
{
    unsigned s  = uv.s(); 
    assert(s < par_nsurf());

    float r1_ = radius();
    float r2_ = radius();
    float z1_ = z1();
    float z2_ = z2();

    assert( z2_ > z1_ );

    glm::vec3 pos(0,0,0);
    pos.x = x();
    pos.y = y();
    // start on axis

    switch(s)
    {
       case 0:  ncone::_par_pos_body(  pos, uv, r1_ ,  z1_ , r2_ , z2_ ) ; break ; 
       case 1:  nnode::_par_pos_endcap(pos, uv, r2_ ,  z2_ )             ; break ; 
       case 2:  nnode::_par_pos_endcap(pos, uv, r1_ ,  z1_ )             ; break ; 
    }
    return pos ; 
}




/*

Can SDFs model finite open cylinder, ie no endcaps or 1 endcap  ?
====================================================================

* i do not think so...  but then CSG cannot do this either

* suspect this is fundamental limitation of geometry modelling with SDF,
  ... **can only handle closed geometry** 

  * yep that is definitiely the case for CSG  *S* means SOLID,


Extract from env-;sdf-:

Slab is a difference of half-spaces

* sdfA = z - h      (plane at z = h) 
* sdfB = z + h      (plane at z = -h ),  
* ~sdfB = -(z+h)    (same position, but now inside are upwards to +z)

::

    intersect(sdfA, ~sdfB) 
    max( z - h , -(z + h) )
    max( z - h , -z - h )
    max(z, -z) - h
    abs(z) - h 


http://iquilezles.org/www/articles/distfunctions/distfunctions.htm

float sdCappedCylinder( vec3 p, vec2 h )
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - h;
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

http://mercury.sexy/hg_sdf/
http://aka-san.halcy.de/distance_fields_prefinal.pdf

By using CSG operations, we can now cut parts of the (infinite) cylinder 
by intersecting it and an infinite z-slab, resulting in a finite z-oriented 
cylinder with radius r and height h:

    d = max( sqrt(px^2+py^2) - r, |pz|-(h/2) )


* "max" corresponds to CSG intersection with z-slab (infinite in x and y)
   which is represented by 

    d = |pz| - (h/2)       <- d = 0 at  pz = +- h/2

*/


