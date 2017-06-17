

#include "NGLMExt.hpp"
#include "GLMFormat.hpp"

#include <glm/gtc/epsilon.hpp>


#include "PLOG.hh"

void test_stream()
{
    glm::ivec3 iv[4] = {
       {1,2,3},
       {10,20,30},
       {100,200,300},
       {1000,2000,3},
    };

    for(int i=0 ; i < 4 ; i++)
        std::cout << std::setw(20) << glm::to_string(iv[i]) << std::endl ; 

    for(int i=0 ; i < 4 ; i++)
        std::cout << std::setw(20) << iv[i] << std::endl ; 



    glm::vec3 fv[4] = {
       {1.23,2.45,3},
       {10.12345,20,30.2235263},
       {100,200,300},
       {1000,2000,3},
    };

    for(int i=0 ; i < 4 ; i++)
        std::cout << std::setw(20) << glm::to_string(fv[i]) << std::endl ; 

    for(int i=0 ; i < 4 ; i++)
        std::cout << std::setw(20) << fv[i] << std::endl ; 

}


void test_invert_tr()
{
    LOG(info) << "test_invert_tr" ;
 
    glm::vec3 axis(1,1,1);
    glm::vec3 tlat(0,0,100) ; 
    float angle = 45.f ; 

    glm::mat4 tr(1.f) ;
    tr = glm::translate(tr, tlat );
    tr = glm::rotate(tr, angle, axis );

    glm::mat4 irit = nglmext::invert_tr( tr );

    //std::cout << gpresent(" tr ", tr) << std::endl ; 
    //std::cout << gpresent(" irit ", irit ) << std::endl ; 

    nmat4pair mp(tr, irit);
    std::cout << " mp " << mp << " dig " << mp.digest() << std::endl ; 

}


void test_make_transform()
{
    LOG(info) << "test_make_transform" ;

    glm::vec3 tlat(0,0,100) ; 
    glm::vec4 trot(0,0,1,45) ; 
    glm::vec3 tsca(1,1,2) ; 

    glm::mat4 srt = nglmext::make_transform("srt", tlat, trot, tsca );
    glm::mat4 trs = nglmext::make_transform("trs", tlat, trot, tsca );

    glm::vec4 orig(0,0,0,1);
    glm::vec4 px(1,0,0,1);
    glm::vec4 py(0,1,0,1);
    glm::vec4 pz(0,0,1,1);
    glm::vec4 pxyz(1,1,1,1);

    std::cout 
         << std::endl 
         << "NB gpresent(mat4) does i=0..3 j=0..3  ( flip ? m[j][i] : m[i][j] )  with flip=false  ie row-by-row ROWMAJOR ? "   
         << std::endl 
         << std::endl 

         << gpresent( "tlat",  tlat ) 
         << gpresent( "trot",  trot ) 
         << gpresent( "tsca",  tsca )

         << std::endl 
        
         << gpresent( "srt",  srt ) << std::endl
         << gpresent( "trs",  trs ) << std::endl 

         << gpresent( "orig",  orig ) 
         << gpresent( "px",  px ) 
         << gpresent( "py",  py ) 
         << gpresent( "pz",  pz ) 
         << gpresent( "pxyz",  pxyz ) 

         << std::endl
         << " z-values ~200 shows the scaling of z by 2 happened after the translate " 
         << std::endl 

         << gpresent( "srt*orig",  srt*orig )  
         << gpresent( "srt*px",    srt*px  ) 
         << gpresent( "srt*py",    srt*py  ) 
         << gpresent( "srt*pz",    srt*pz  )
         << gpresent( "srt*pxyz",  srt*pxyz  )

         << std::endl 
         << " z-values ~100 shows the scaling of z by 2 happened before the translate " 
         << std::endl 


         << gpresent( "trs*orig",  trs*orig ) 
         << gpresent( "trs*px",    trs*px  ) 
         << gpresent( "trs*py",    trs*py  )
         << gpresent( "trs*pz",    trs*pz  )
         << gpresent( "trs*pxyz",  trs*pxyz  )

         << std::endl 
         << " wrong way around gives crazy w for some" 
         << std::endl 

         << gpresent( "orig*srt",  orig*srt ) 
         << gpresent( "px*srt",    px*srt  ) 
         << gpresent( "py*srt",    py*srt  ) 
         << gpresent( "pz*srt",    pz*srt  )
         << gpresent( "pxyz*srt",  pxyz*srt  )

         << std::endl 

         << gpresent( "orig*trs ",  orig*trs ) 
         << gpresent( "px*trs",     px*trs  ) 
         << gpresent( "py*trs",     py*trs  )
         << gpresent( "pz*trs",     pz*trs  )
         << gpresent( "pxyz*trs",   pxyz*trs  )

         << std::endl 
         ;  
}



void test_nmat4triple_make_translated()
{
    LOG(info) << "test_nmat4triple_make_translated" ;

    glm::vec3 tlat(0,0,100) ; 
    glm::vec4 trot(0,0,1,0) ; 
    glm::vec3 tsca(1,1,2) ; 

    glm::mat4 trs = nglmext::make_transform("trs", tlat, trot, tsca );

    nmat4triple tt0(trs);

    glm::vec3 tlat2(-100,0,0) ; 

    bool reverse(true);
    nmat4triple* tt1 = tt0.make_translated( tlat2, reverse, "test_nmat4triple_make_translated" );
    nmat4triple* tt2 = tt0.make_translated( tlat2, !reverse, "test_nmat4triple_make_translated" );

    std::cout 
         << std::endl 
         << gpresent("trs", trs) 
         << std::endl 
         << gpresent("tt0.t", tt0.t ) 
         << std::endl 
         << gpresent("tt1.t", tt1->t ) 
         << std::endl 
         << gpresent("tt2.t", tt2->t ) 
         << std::endl 
        ;  
}


void test_nmat4triple_id_digest()
{
    LOG(info) << "test_nmat4triple_id_digest" ; 

    nmat4triple* id = nmat4triple::make_identity() ;
    std::cout << " id " << *id << " dig " << id->digest() << std::endl ; 
}



int main(int argc, char** argv)
{
    PLOG_(argc, argv) ; 

    //test_stream();
    //test_invert_tr();
    test_make_transform();
    test_nmat4triple_make_translated();
    //test_nmat4triple_id_digest();

    return 0 ; 
}
