// ggv --lookup
// ggv --jpmt --lookup


#include "NLookup.hpp"
#include "Opticks.hh"
#include "GBndLib.hh"


#include "GGEO_BODY.hh"
#include "PLOG.hh"


/*

ChromaMaterialMap.json contains name to code mappings used 
for a some very old gensteps that were produced by G4DAEChroma
and which are still in use.
As the assumption of all gensteps being produced the same
way and with the same material mappings will soon become 
incorrect, need a more flexible way.

Perhaps a sidecar file to the gensteps .npy should
contain material mappings, and if it doesnt exist then 
defaults are used ?

::

    simon:DayaBay blyth$ cat ChromaMaterialMap.json | tr "," "\n"
    {"/dd/Materials/OpaqueVacuum": 18
     "/dd/Materials/Pyrex": 21
     "/dd/Materials/PVC": 20
     "/dd/Materials/NitrogenGas": 16
     "/dd/Materials/Teflon": 24
     "/dd/Materials/ESR": 9
     "/dd/Materials/MineralOil": 14
     "/dd/Materials/Vacuum": 27
     "/dd/Materials/Bialkali": 5
     "/dd/Materials/Air": 2
     "/dd/Materials/OwsWater": 19
     "/dd/Materials/C_13": 6
     "/dd/Materials/IwsWater": 12
     "/dd/Materials/ADTableStainlessSteel": 0
     "/dd/Materials/Ge_68": 11
     "/dd/Materials/Acrylic": 1
     "/dd/Materials/Tyvek": 25
     "/dd/Materials/Water": 28
     "/dd/Materials/Nylon": 17
     "/dd/Materials/LiquidScintillator": 13
     "/dd/Materials/GdDopedLS": 10
     "/dd/Materials/UnstStainlessSteel": 26
     "/dd/Materials/BPE": 4
     "/dd/Materials/Silver": 22
     "/dd/Materials/DeadWater": 8
     "/dd/Materials/Co_60": 7
     "/dd/Materials/Aluminium": 3
     "/dd/Materials/Nitrogen": 15
     "/dd/Materials/StainlessSteel": 23}

*/


int main(int argc, char** argv)
{
    PLOG_(argc, argv);

    Opticks* m_opticks = new Opticks(argc, argv);

    GBndLib* blib = GBndLib::load(m_opticks, true );

    blib->dump();


    NLookup* m_lookup = new NLookup();

    const char* cmmd = m_opticks->getDetectorBase() ;

    m_lookup->loadA( cmmd , "ChromaMaterialMap.json", "/dd/Materials/") ;

    std::map<std::string, unsigned int>& B = m_lookup->getB() ;

    blib->fillMaterialLineMap( B ) ;     // shortname eg "GdDopedLS" to material line mapping 

    m_lookup->crossReference();

    m_lookup->dump("ggeo-/NLookupTest");


    printf("  a => b \n");
    for(unsigned int a=0; a < 35 ; a++ )
    {   
        int b = m_lookup->a2b(a);
        std::string aname = m_lookup->acode2name(a) ;
        std::string bname = m_lookup->bcode2name(b) ;
        printf("  %3u -> %3d  ", a, b );

        if(b < 0) 
        {
            LOG(warning) 
                       << " FAILED TO TRANSLATE a->b "
                       << " a " << std::setw(3) << a 
                       << " b " << std::setw(3) << b
                       << " an " << std::setw(25) << aname 
                       << " bn " << std::setw(25) << bname 
                       ;
        } 
        else
        {   
             assert(aname == bname);
             printf(" %25s \n", aname.c_str() );
        }   
    }   
}



