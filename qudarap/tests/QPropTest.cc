#include "SPath.hh"
#include "NP.hh"
#include "Opticks.hh"
#include "QProp.hh"
#include "OPTICKS_LOG.hh"

const char* FOLD = "/tmp/QPropTest" ; 

/**
test_lookup
-------------

nx lookups in x0->x1 inclusive for each property yielding nx*qp.ni values.

**/

template <typename T>
void test_lookup(const QProp<T>& qp, T x0, T x1, T nx, const char* reldir)
{
    NP* x = NP::Linspace<T>( x0, x1, nx ); 
    NP* y = NP::Make<T>(qp.ni, nx ); 

    qp.lookup(y->values<T>(), x->cvalues<T>(), qp.ni, nx );

    qp.a->save(FOLD, reldir, "prop.npy"); 
    x->save(FOLD,    reldir, "domain.npy"); 
    y->save(FOLD,    reldir, "lookup.npy"); 

    LOG(info) << "save to " << FOLD << "/" << reldir  ; 
}


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    Opticks ok(argc, argv); 
    ok.configure(); 

    const char* idpath = ok.getIdPath(); 
    int create_dirs = 0 ; 
    const char* rindexpath = SPath::Resolve(idpath, "GScintillatorLib/LS_ori/RINDEX.npy", create_dirs ); 
    LOG(info) << " rindexpath " << rindexpath ; 



    unsigned nx = 1601u ; 
    //unsigned nx = 161u ; 

    //QProp<float> qpf(rindexpath) ; 
    //test_lookup<float>(qpf, 0.f, 16.f, nx , "float" );

    

    QProp<double> qpd(rindexpath) ; 
    test_lookup<double>(qpd, 0., 16., nx , "double" );


    return 0 ; 
}

