
#include <cstdio>
#include <cassert>
#include <cmath>
#include <sstream>
#include <iomanip>

#include "NGLM.hpp"

#include "NNode.hpp"
#include "NPart.hpp"
#include "NQuad.hpp"
#include "NSphere.hpp"
#include "NBox.hpp"
#include "NBBox.hpp"

#include "PLOG.hh"


double nnode::operator()(double,double,double) 
{
    return 0.f ; 
} 

std::string nnode::desc()
{
    std::stringstream ss ; 
    ss  << " nnode "
        << std::setw(3) << type 
        << std::setw(15) << CSGName(type) 
        ;     
    return ss.str();
}


void nnode::dump(const char* msg)
{
    printf("(%s)%s\n",csgname(), msg);
    if(left && right)
    {
        left->dump("left");
        right->dump("right");
    }

    if(transform)
    {
        std::cout << "transform: " << glm::to_string( *transform ) << std::endl ; 
    } 

}

void nnode::Init( nnode& n , OpticksCSG_t type, nnode* left, nnode* right )
{
    n.type = type ; 

    n.left = left ; 
    n.right = right ; 
    n.parent = NULL ; 

    n.transform = NULL ; 
    n.gtransform = NULL ; 

    n.param = {0.f, 0.f, 0.f, 0.f };
}

const char* nnode::csgname()
{
   return CSGName(type);
}
unsigned nnode::maxdepth()
{
    return _maxdepth(0);
}
unsigned nnode::_maxdepth(unsigned depth)  // recursive 
{
    return left && right ? nmaxu( left->_maxdepth(depth+1), right->_maxdepth(depth+1)) : depth ;  
}


glm::mat4* nnode::global_transform()
{
    glm::mat4 gt ; 

    nnode* n = this ; 
    int ntra = 0 ; 
    while(n)
    {
        if(n->transform)
        {
           ntra++ ; 
           gt *= *n->transform ;   // is this the correct order of matrix multiplication ?
        }
        n = n->parent ; 
    }
    return ntra == 0 ? NULL : new glm::mat4(gt) ; 
}



npart nnode::part()
{
    nbbox bb = bbox();

    npart pt ; 
    pt.zero();
    pt.setParam( param );
    pt.setTypeCode( type );
    pt.setBBox( bb );

    return pt ; 
}


nbbox nnode::bbox()
{
   // needs to be overridden for primitives
    nbbox bb = make_nbbox() ; 
    if(left && right)
    {
        bb.include( left->bbox() );
        bb.include( right->bbox() );
    }
    return bb ; 
}



/**
To translate or rotate a surface modeled as an SDF, you can apply the inverse
transformation to the point before evaluating the SDF.

**/


double nunion::operator()(double px, double py, double pz) 
{
    assert( left && right );
    double l = (*left)(px, py, pz) ;
    double r = (*right)(px, py, pz) ;
    return fmin(l, r);
}
double nintersection::operator()(double px, double py, double pz) 
{
    assert( left && right );
    double l = (*left)(px, py, pz) ;
    double r = (*right)(px, py, pz) ;
    return fmax( l, r);
}
double ndifference::operator()(double px, double py, double pz) 
{
    assert( left && right );
    double l = (*left)(px, py, pz) ;
    double r = (*right)(px, py, pz) ;
    return fmax( l, -r);    // difference is intersection with complement, complement negates signed distance function
}


void nnode::Tests(std::vector<nnode*>& nodes )
{
    nsphere* a = make_nsphere_ptr(0.f,0.f,-50.f,100.f);
    nsphere* b = make_nsphere_ptr(0.f,0.f, 50.f,100.f);
    nbox*    c = make_nbox_ptr(0.f,0.f,0.f,200.f);

    nunion* u = make_nunion_ptr( a, b );
    nintersection* i = make_nintersection_ptr( a, b ); 
    ndifference* d1 = make_ndifference_ptr( a, b ); 
    ndifference* d2 = make_ndifference_ptr( b, a ); 
    nunion* u2 = make_nunion_ptr( d1, d2 );

    nodes.push_back( (nnode*)a );
    nodes.push_back( (nnode*)b );
    nodes.push_back( (nnode*)u );
    nodes.push_back( (nnode*)i );
    nodes.push_back( (nnode*)d1 );
    nodes.push_back( (nnode*)d2 );
    nodes.push_back( (nnode*)u2 );

    nodes.push_back( (nnode*)c );


    float radius = 200.f ; 
    float inscribe = 1.3f*radius/sqrt(3.f) ; 

    nsphere* sp = make_nsphere_ptr(0.f,0.f,0.f,radius);
    nbox*    bx = make_nbox_ptr(0.f,0.f,0.f, inscribe );
    nunion*  u_sp_bx = make_nunion_ptr( sp, bx );
    nintersection*  i_sp_bx = make_nintersection_ptr( sp, bx );
    ndifference*    d_sp_bx = make_ndifference_ptr( sp, bx );
    ndifference*    d_bx_sp = make_ndifference_ptr( bx, sp );

    nodes.push_back( (nnode*)u_sp_bx );
    nodes.push_back( (nnode*)i_sp_bx );
    nodes.push_back( (nnode*)d_sp_bx );
    nodes.push_back( (nnode*)d_bx_sp );


}




std::function<float(float,float,float)> nnode::sdf()
{
    nnode* node = this ; 
    std::function<float(float,float,float)> f ; 
    switch(node->type)
    {
        case CSG_UNION:
            {
                nunion* n = (nunion*)node ; 
                f = *n ;
            }
            break ;
        case CSG_INTERSECTION:
            {
                nintersection* n = (nintersection*)node ; 
                f = *n ;
            }
            break ;
        case CSG_DIFFERENCE:
            {
                ndifference* n = (ndifference*)node ; 
                f = *n ;
            }
            break ;
        case CSG_SPHERE:
            {
                nsphere* n = (nsphere*)node ; 
                f = *n ;
            }
            break ;
        case CSG_BOX:
            {
                nbox* n = (nbox*)node ;  
                f = *n ;
            }
            break ;
        default:
            LOG(fatal) << "Need to add upcasting for type: " << node->type << " name " << CSGName(node->type) ;  
            assert(0);
    }
    return f ;
}


