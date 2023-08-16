#include "U4Polycone.h"
#include "G4SystemOfUnits.hh"

#include "NPFold.h"

#ifdef WITH_SND
#include "scsg.hh"
#else
#include "s_csg.h"
#endif

/**

                     :
        +------------:-------------+
        |            :             |
        +-----+      :      +------+ 
              |      :      |
              +------:------+
                     :
**/


int main()
{
    G4double phiStart = 0.00*deg ; 
    G4double phiTotal = 360.00*deg ; 

    G4int numRZ = 4 ; 
    G4double ri[] = {0. ,  0.,  0. , 0.    } ; 
    //G4double ro[] = {50. , 50., 100., 100. } ; 
    G4double ro[] = {264.050, 264.050, 264.050, 132.025 } ; 
    G4double z[] = { -183.225,      0., 100.  , 200.050 } ; 
    
    G4Polycone* pc = new G4Polycone("name", phiStart, phiTotal, numRZ, z, ri, ro ); 
    G4cout << *pc << std::endl ; 

    int level = 1 ; 

    NPFold* fold = nullptr ; 
#ifdef WITH_SND
    scsg* csg = new scsg ; 
    assert(csg); 

    int root = U4Polycone::Convert( pc, level );  
    std::cout <<  snd::Render(root) ; 
    std::cout << csg->brief() << std::endl ;  
    std::cout << csg->desc() << std::endl ;  

    fold = csg->serialize(); 
    fold->save("$FOLD/csg");
#else
    s_csg* csg = new s_csg ; 
    assert(csg); 

    sn* root = U4Polycone::Convert( pc, level );  
    std::cout << root->render() ;  


    std::cout << csg->brief() << std::endl ;  
    fold = csg->serialize(); 
    fold->save("$FOLD/_csg");
#endif
    return 0 ; 
}
