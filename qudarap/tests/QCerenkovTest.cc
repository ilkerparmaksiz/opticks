
#include "SPath.hh"
#include "NP.hh"
#include "QCerenkov.hh"
#include "QCK.hh"

#include "scuda.h"
#include "OPTICKS_LOG.hh"


void test_check(QCerenkov& ck)
{
    ck.check(); 
}

void test_lookup(QCerenkov& ck)
{
    NP* dst = ck.lookup(); 
    const char* fold = "$TMP/QCerenkovTest" ; 
    LOG(info) << " save to " << fold ; 
    dst->save(fold, "dst.npy"); 
    ck.src->save(fold, "src.npy") ; 
}

/**
test_GetAverageNumberOfPhotons_s2
-----------------------------------

See ana/ckn.py:compareOtherScans for check that these
results match those from the python prototype and cks 

**/

void test_GetAverageNumberOfPhotons_s2(const QCerenkov& ck)
{
    LOG(info) ; 
    const double charge = 1. ; 
    double numPhotons, emin, emax ; 
     
    unsigned ni = 1001u ; 
    NP* bis = NP::Linspace<double>(1., 2., ni ); 
    const double* bb = bis->cvalues<double>(); 

    NP* scan = NP::Make<double>(ni, 4);   
    double* ss = scan->values<double>();  

    for(unsigned i=0 ; i < unsigned(bis->shape[0]) ; i++ )
    {
        const double BetaInverse = bb[i] ; 
        numPhotons = ck.GetAverageNumberOfPhotons_s2<double>(emin, emax, BetaInverse, charge ); 

        ss[4*i+0] = BetaInverse ; 
        ss[4*i+1] = numPhotons ; 
        ss[4*i+2] = emin ; 
        ss[4*i+3] = emax ; 

        if(numPhotons != 0.) 
            std::cout  
                << " i " << std::setw(5) << i 
                << " BetaInverse " << std::fixed << std::setw(10) << std::setprecision(4) << BetaInverse
                << " numPhotons " << std::fixed << std::setw(10) << std::setprecision(4) << numPhotons
                << " emin " << std::fixed << std::setw(10) << std::setprecision(4) << emin
                << " emax " << std::fixed << std::setw(10) << std::setprecision(4) << emax
                << std::endl 
                ; 
    }

    const char* path = SPath::Resolve("$TMP/QCerenkovTest/test_GetAverageNumberOfPhotons_s2.npy"); 
    LOG(info) << " save to " << path ; 
    scan->save(path); 
}


void test_getS2CumulativeIntegrals_one( const QCerenkov& ck, double BetaInverse )
{
    unsigned nx = 100 ; 
    NP* s2c = ck.getS2CumulativeIntegrals(BetaInverse, nx); 

    LOG(info) 
        << " BetaInverse " <<  std::fixed << std::setw(10) << std::setprecision(4) << BetaInverse
        << " s2c " << s2c->desc()
        ; 

    const char* path = SPath::Resolve("$TMP/QCerenkovTest/test_getS2CumulativeIntegrals_one.npy"); 
    LOG(info) << " save to " << path ; 
    s2c->save(path); 
}




void test_getS2CumulativeIntegrals_many(const QCerenkov& ck )
{
    const NP* bis  = NP::Linspace<double>( 1., 2. , 1001 );     // BetaInverse
    unsigned nx = 100u ; 
    NP* s2c = ck.getS2CumulativeIntegrals<double>(bis, nx ); 

 
    LOG(info) 
        << std::endl
        << " bis    " << bis->desc()
        << std::endl
        << " s2c " << s2c->desc()
        ; 


    const char* fold = SPath::Resolve("$TMP/QCerenkovTest/test_getS2CumulativeIntegrals"); 
    SPath::MakeDirs(fold); 
    LOG(info) << " save to " << fold ; 
    s2c->save(fold,"s2c.npy"); 

    s2c->divide_by_last<double>();  
    s2c->save(fold,"s2cn.npy"); 
}


void test_getS2Integral_Cumulative( const QCerenkov& ck, const char* bis_, unsigned mul )
{
    const NP* bis  = bis_ == nullptr ? NP::Linspace<double>( 1., 2. , 1001 ) : NP::FromString<double>(bis_);     

    NP* s2c = ck.getS2Integral_Cumulative<double>(bis, mul); 

    LOG(info) 
        << std::endl
        << " bis " << bis->desc()
        << std::endl
        << " s2c " << s2c->desc()
        << " mul " << mul 
        ; 

    const char* fold = SPath::Resolve("$TMP/QCerenkovTest/test_getS2Integral_Cumulative"); 
    SPath::MakeDirs(fold); 
    LOG(info) << " save to " << fold ; 
    s2c->save(fold, "s2c.npy"); 

    s2c->divide_by_last<double>();  
    s2c->save(fold, "s2cn.npy"); 
}


void test_makeICDF(const QCerenkov& ck, unsigned ny, unsigned nx )
{
    QCK<double> icdf = ck.makeICDF<double>( ny, nx ); 

    LOG(info)
        << std::endl  
        << " icdf.bis  " << icdf.bis->desc()
        << std::endl  
        << " icdf.s2c  " << icdf.s2c->desc() 
        << std::endl  
        << " icdf.s2cn " << icdf.s2cn->desc()
        << std::endl  
        ;

    const char* icdf_path = SPath::Resolve("$TMP/QCerenkovTest/test_makeICDF"); 
    int rc = SPath::MakeDirs(icdf_path);   
    assert( rc == 0 ); 
    icdf.save(icdf_path); 
}




int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    QCerenkov ck ;  
    //const double BetaInverse = 1.0 ; 

    //test_lookup(ck); 
    //test_check(ck); 

    //test_GetAverageNumberOfPhotons_s2(ck); 

    //test_getS2CumulativeIntegrals_one(ck,BetaInverse) ; 
    //test_getS2CumulativeIntegrals_many(ck) ; 

    //const char* bis = "1.0 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.792" ; 
    const char* bis = "1.0" ; 
    unsigned mul = 2 ; 
    test_getS2Integral_Cumulative(ck, bis, mul ) ; 

    //unsigned ny = 2000u ; 
    //unsigned nx = 2000u ; 
    //test_makeICDF(ck, ny, nx ); 


    return 0 ; 
}

