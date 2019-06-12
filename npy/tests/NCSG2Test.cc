// TEST=NCSG2Test om-t

/**
**/

#include <iostream>

#include "BFile.hh"
#include "NCSG.hpp"
#include "NNode.hpp"
#include "NBBox.hpp"
#include "NGLMExt.hpp"
#include "NQuad.hpp"
#include "GLMFormat.hpp"

#include "OPTICKS_LOG.hh"

#include "BOpticks.hh"


void test_apply_centering_0( NCSG* csg )  // NB this code now moved into nnode::apply_centering 
{
    nbbox bb0 = csg->bbox() ; 
    nvec4 ce0 = bb0.center_extent() ;
    bool centered0 = ce0.x == 0.f && ce0.y == 0.f && ce0.z == 0.f ; 

    LOG(info) 
        << " bb0 " << bb0.description()
        << " ce0 " << ce0.desc()
        << ( centered0 ? " CENTERED " : " NOT-CENTERED " )
        ; 

    if(!centered0  )
    {
        nnode* root = csg->getRoot(); 
        LOG(info) << " root->transform " << *root->transform ;  
        root->placement = nmat4triple::make_translate( -ce0.x, -ce0.y, -ce0.z );  
        root->update_gtransforms(); 
    } 

    nbbox bb1 = csg->bbox();  // <-- global frame bbox, even for single primitive 
    nvec4 ce1 = bb1.center_extent() ;
    bool centered1 = ce1.x == 0.f && ce1.y == 0.f && ce1.z == 0.f ; 

    LOG(info) 
        << " bb1 " << bb1.description()
        << " ce1 " << ce1.desc()
        << ( centered1 ? " CENTERED " : " NOT-CENTERED " )
        ; 

    assert( centered1 ); 
}


void test_apply_centering_1( NCSG* csg )
{
    nnode* root = csg->getRoot(); 
    root->verbosity = 1 ; 
    root->apply_centering(); 
    root->verbosity = 0 ; 
}


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    BOpticks ok(argc, argv, "--envkey" );  
    if(ok.getError() > 0) return 0 ;

    const char* lvid = ok.getFirstArg("17"); 
    const char* path = ok.getPath("GMeshLibNCSG", lvid ); 

    LOG(info) << "[" << path  << "]" ;  

    NCSG* csg = NCSG::Load(path); 
    if(!csg) return 0 ; 

    //test_apply_centering_0(csg); 
    test_apply_centering_1(csg); 

    return 0 ; 
}


