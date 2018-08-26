// TEST=CAlignEngineTest om-t 

#include <iostream>
#include <iomanip>

#include "OPTICKS_LOG.hh"
#include "CAlignEngine.hh"
#include "NPY.hpp"
#include "Randomize.hh"


struct CAlignEngineTest
{
    CAlignEngineTest();  
    void check( int v0 , int v1, int i0, int i1, bool dump );

    NPY<double>* seq ; 

};


CAlignEngineTest::CAlignEngineTest()
{
    CAlignEngine::SetSequenceIndex(-1); 

    const CAlignEngine* ae = CAlignEngine::INSTANCE ; 
    seq = ae->m_seq ;  
    assert( seq ); 

    assert( ae->m_seq_nv == 256 );      
    assert( ae->m_seq_ni == 100000 );      

    int ni =  ae->m_seq_ni ; 
    int nv =  ae->m_seq_nv ; 

    LOG(info) 
        << " ni " << ni 
        << " nv " << nv
        ;

    //check( 0, 16, 0, 10, true ); 
    check(  0,  nv, 0, 10, true );     

    
}


void CAlignEngineTest::check( int v0 , int v1, int i0, int i1, bool dump )
{
    // for each value in the sequence hop between streams

    assert( v0 == 0 && " have to start from value zero for the check to match "); 

    // are not accessing something with an index, 
    // are making a sequence of calls to G4UniformRand()

    if(dump)
    {
        std::cout << std::setw(10) << "" << "   " ; 
        for(int i=i0 ; i < i1 ; i++) std::cout << " " <<  std::setw(10) << i ; 
        std::cout << std::endl ; 
    } 

    for(int v=v0 ; v < v1 ; v++)
    {
        if(dump)
            std::cout << std::setw(10) << v << " : " ; 


        for(int i=i0 ; i < i1 ; i++)
        {

            CAlignEngine::SetSequenceIndex(i); 

            double u = G4UniformRand() ; 

            if(dump) std::cout << " " << std::setw(10) << std::fixed << std::setprecision(6) << u  ;
            if( i >= 0 )
            {
                double u2 = seq->getValue(i, 0, 0, v);
                //if(dump) std::cout << " (" << std::setw(10) << std::fixed << std::setprecision(6) << u2 << ")" ; 

                bool match = u == u2 ; 
                if(!match)
                    LOG(fatal) 
                       << " mismatch "
                       << " v " << v
                       << " i " << i
                       << " u " << u 
                       << " u2 " << u2
                       ; 

                assert(match); 
            }
        }
        if(dump) std::cout << std::endl  ;
    } 
}


int main( int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    CAlignEngineTest aet ; 

    return 0 ; 
}
