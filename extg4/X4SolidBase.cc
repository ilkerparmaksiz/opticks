#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>


#include "SStr.hh"
#include "NNode.hpp"
#include "X4Solid.hh"
#include "G4VSolid.hh"
#include "PLOG.hh"

// generated by x4-include- Mon Jun 11 20:24:54 HKT 2018 
#include "G4BooleanSolid.hh"
#include "G4MultiUnion.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4EllipticalCone.hh"
#include "G4Ellipsoid.hh"
#include "G4EllipticalTube.hh"
#include "G4ExtrudedSolid.hh"
#include "G4Hype.hh"
#include "G4Orb.hh"
#include "G4Para.hh"
#include "G4Paraboloid.hh"
#include "G4Polycone.hh"
#include "G4GenericPolycone.hh"
#include "G4Polyhedra.hh"
#include "G4Sphere.hh"
#include "G4TessellatedSolid.hh"
#include "G4Tet.hh"
#include "G4Torus.hh"
#include "G4GenericTrap.hh"
#include "G4Trap.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4CutTubs.hh"
#include "G4TwistedBox.hh"
#include "G4TwistedTrap.hh"
#include "G4TwistedTrd.hh"
#include "G4TwistedTubs.hh"

#include "SId.hh"

SId* X4SolidBase::NODE_ID = new SId("abcdefghijklmnopqrstuvwxyz") ; 
SId* X4SolidBase::OTHER_ID = new SId("ABCDEFGHIJKLMNOPQRSTUVWXYZ") ; 

/*
int X4SolidBase::IDENTIFIER_IDX = -1 ; 
void X4SolidBase::ResetIdentifier()
{
    IDENTIFIER_IDX = -1 ; 
}


const char* X4SolidBase::Identifier(bool reset)
{
    if(reset) ResetIdentifier(); 
    IDENTIFIER_IDX += 1 ;  
    int idx = IDENTIFIER_IDX ; 
    assert( idx < int(strlen(IDENTIFIERS)) );
    char id = IDENTIFIERS[idx] ; 
    return strdup(&id) ; 
}
*/



std::string X4SolidBase::desc() const 
{
    std::stringstream ss ; 
    ss << "X4SolidBase" 
       << " name " << std::setw(40) << m_name
       << " entityType " << m_entityType 
       << " entityName " << m_entityName
       << " root " << m_root
       ;
    return ss.str();
}

G4Hype* X4SolidBase::MakeHyperboloid(const char* name, float rmin , float rmax, float inst, float outst, float hz )
{
    G4double  newInnerRadius = rmin ; 
    G4double  newOuterRadius = rmax ; 
    G4double  newInnerStereo = inst*CLHEP::pi/180. ; 
    G4double  newOuterStereo = outst*CLHEP::pi/180. ; 
    G4double  newHalfLenZ    = hz ; 
    return new G4Hype( name, newInnerRadius, newOuterRadius, newInnerStereo, newOuterStereo, newHalfLenZ ); 
}

G4Ellipsoid* X4SolidBase::MakeEllipsoid(const char* name, float ax, float by, float cz, float zcut1, float zcut2  )
{
    G4double  pxSemiAxis = ax ; 
    G4double  pySemiAxis = by ; 
    G4double  pzSemiAxis = cz ; 
    G4double  pzBottomCut = zcut1 ; 
    G4double  pzTopCut = zcut2 ; 
    return new G4Ellipsoid( name, pxSemiAxis, pySemiAxis, pzSemiAxis, pzBottomCut, pzTopCut ); 
}


G4Torus* X4SolidBase::MakeTorus(const char* name, float R, float r )
{
    G4double Rmin = 0. ; 
    G4double Rmax = r ; 
    G4double Rtor = R ; 
    G4double SPhi = 0. ; 
    G4double DPhi = 2.0*CLHEP::pi ; 
    return new G4Torus( name, Rmin, Rmax, Rtor, SPhi, DPhi ); 
}


G4Cons* X4SolidBase::MakeCone(const char* name, float z, float rmax1, float rmax2, float rmin1, float rmin2, float startphi, float deltaphi  )
{
    G4double pRmin1 = rmin1 ; 
    G4double pRmax1 = rmax1 ; 
    G4double pRmin2 = rmin2 ; 
    G4double pRmax2 = rmax2 ; 
    G4double pDz    = z/2.0 ; 
    G4double pSPhi  = startphi*CLHEP::pi/180. ; 
    G4double pDPhi  = deltaphi*CLHEP::pi/180. ;
    return new G4Cons( name, pRmin1, pRmax1, pRmin2, pRmax2, pDz, pSPhi, pDPhi  );
}

G4Trd* X4SolidBase::MakeTrapezoidCube(const char* name, float sz)
{
    return MakeTrapezoid( name, sz, sz, sz, sz, sz ); 
}

G4Trd* X4SolidBase::MakeTrapezoid(const char* name, float z,  float x1, float y1, float x2, float y2 )
{
    return new G4Trd(name, z, x1, y1, x2, y2 ); 
} 
G4Tubs* X4SolidBase::MakeTubs(const char* name, float rmin, float rmax, float hz, float startphi, float deltaphi )
{
    G4double pSPhi = startphi*CLHEP::pi/180. ; 
    G4double pDPhi = deltaphi*CLHEP::pi/180. ;

    return new G4Tubs(name, rmin, rmax, hz, pSPhi, pDPhi ); 
}

G4Orb* X4SolidBase::MakeOrb(const char* name, float radius )
{
    return new G4Orb(name, radius); 
}

G4Sphere* X4SolidBase::MakeZSphere(const char* name, float rmin, float rmax, float startPhi, float deltaPhi, float startTheta, float deltaTheta )
{
    G4String name_(name);
    G4double pRmin = rmin ; 
    G4double pRmax = rmax ; 

    G4double pSPhi = startPhi*CLHEP::pi/180. ; 
    G4double pDPhi = deltaPhi*CLHEP::pi/180. ;

    G4double pSTheta = startTheta*CLHEP::pi/180. ; 
    G4double pDTheta = deltaTheta*CLHEP::pi/180. ;

    G4Sphere* sp = new G4Sphere(name_, pRmin, pRmax, pSPhi, pDPhi, pSTheta, pDTheta );
    return sp ; 
}

G4Sphere* X4SolidBase::MakeSphere(const char* name, float rmax, float rmin)
{
   return MakeZSphere(name, rmin, rmax, 0.f, 360.f, 0.f, 180.f ); 

}

X4SolidBase::X4SolidBase( const G4VSolid* solid, bool top )  
   :
   m_solid(solid),
   m_top(top),
   m_name(strdup(solid->GetName().c_str())),
   m_entityType(X4Entity::Type(solid->GetEntityType())),
   m_entityName(X4Entity::Name(m_entityType)),
   m_identifier(NODE_ID->get(top)),
   m_root(NULL)
{
}

template<typename T>
std::string X4SolidBase::GenInstanciate(const char* cls, const char* identifier, const char* name, const std::vector<T>& param) // static
{
    std::stringstream ss ; 
    ss << "G4VSolid* "
       << identifier 
       << " = new "
       << cls
       << "(" 
       <<  "\""
       << name 
       << "\"" 
       << "," 
       ;

    unsigned npar = param.size(); 
    for(unsigned i=0 ; i < npar ; i++) 
    { 
       ss << param[i] ;     
       if( i < npar - 1) ss << " , " ;
    } 

    ss << ") ;" ; 
    return ss.str(); 
}


template std::string X4SolidBase::GenInstanciate<float>(const char*, const char*, const char*, const std::vector<float>& );
template std::string X4SolidBase::GenInstanciate<double>(const char*, const char*, const char*, const std::vector<double>& );
template std::string X4SolidBase::GenInstanciate<std::string>(const char*, const char*, const char*, const std::vector<std::string>& );



template<typename T>
void X4SolidBase::setG4Param(const std::vector<T>& param, const char* identifier_ )
{
    assert( m_root && "must setG4Param after setRoot " ); 

    unsigned npar =param.size() ; 
    if( npar == 0)
       LOG(fatal) << " must setG4Param before generating g4code for instanciation of G4VSolid " 
                  << " for entity " <<  m_entityName
                  ;

    assert( npar > 0 ); 

    const char* identifier = identifier_ ? identifier_ : m_identifier ; 

    std::stringstream ss ; 
    ss << GenInstanciate( m_entityName, identifier, m_name, param ) ;
    std::string g4code = ss.str();
    LOG(info) << "setG4Param " 
              << " npar : " << npar
              << " g4code : " << g4code
              ; 

    setG4Code( g4code.c_str() ); 
}

template void X4SolidBase::setG4Param<float>(const std::vector<float>&, const char* );
template void X4SolidBase::setG4Param<double>(const std::vector<double>&, const char* );
template void X4SolidBase::setG4Param<std::string>(const std::vector<std::string>&, const char* );

const char* X4SolidBase::getIdentifier() const 
{
    return m_identifier ; 
}
 
void X4SolidBase::addG4Code( const char* g4code )
{
    m_g4code.push_back(g4code); 
}

void X4SolidBase::setG4Code( const char* g4code )
{
    assert( m_root && "must setG4Code after setRoot " ); 
    if(m_g4code.size() == 0 )
    {
        m_root->g4code = strdup(g4code) ; 
    }
    else
    {
        addG4Code(g4code); 
        std::stringstream ss ; 
        for( unsigned i=0 ; i < m_g4code.size() ; i++) ss << m_g4code[i] << std::endl ;  
        std::string concat = ss.str(); 
        m_root->g4code = strdup(concat.c_str()) ; 
    }
    LOG(info) << " root.g4code " << m_root->g4code ; 
}

const char* X4SolidBase::getG4Code(const char* identifier) const 
{
    // not using this currently : instead trying to 
    // set identifies at instanciation rather than fixing up lates 
    //
    // Note that NNode used concatenated identifiers for booleans joining 
    // all the primitives identifiers.

    assert( m_root && "must setRoot and setG4Code/setG4Param before this " ); 
    const char* g4code = m_root->g4code ; 
    return SStr::Format1<256>( g4code, identifier ) ; 
}
 
nnode* X4SolidBase::root() const 
{
    return m_root ; 
}
void X4SolidBase::setRoot(nnode* root)
{
    assert( root ) ; 
    m_root = root ; 
}

// generated by x4-convert-cc- Mon Jun 11 20:34:50 HKT 2018 
void X4SolidBase::convertBooleanSolid()
{  
    const G4BooleanSolid* const solid = static_cast<const G4BooleanSolid*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertMultiUnion()
{  
    const G4MultiUnion* const solid = static_cast<const G4MultiUnion*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertBox()
{  
    const G4Box* const solid = static_cast<const G4Box*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertCons()
{  
    const G4Cons* const solid = static_cast<const G4Cons*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertEllipticalCone()
{  
    const G4EllipticalCone* const solid = static_cast<const G4EllipticalCone*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertEllipsoid()
{  
    const G4Ellipsoid* const solid = static_cast<const G4Ellipsoid*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertEllipticalTube()
{  
    const G4EllipticalTube* const solid = static_cast<const G4EllipticalTube*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertExtrudedSolid()
{  
    const G4ExtrudedSolid* const solid = static_cast<const G4ExtrudedSolid*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertHype()
{  
    const G4Hype* const solid = static_cast<const G4Hype*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertOrb()
{  
    const G4Orb* const solid = static_cast<const G4Orb*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertPara()
{  
    const G4Para* const solid = static_cast<const G4Para*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertParaboloid()
{  
    const G4Paraboloid* const solid = static_cast<const G4Paraboloid*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertPolycone()
{  
    const G4Polycone* const solid = static_cast<const G4Polycone*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertGenericPolycone()
{  
    const G4GenericPolycone* const solid = static_cast<const G4GenericPolycone*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertPolyhedra()
{  
    const G4Polyhedra* const solid = static_cast<const G4Polyhedra*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertSphere()
{  
    const G4Sphere* const solid = static_cast<const G4Sphere*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTessellatedSolid()
{  
    const G4TessellatedSolid* const solid = static_cast<const G4TessellatedSolid*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTet()
{  
    const G4Tet* const solid = static_cast<const G4Tet*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTorus()
{  
    const G4Torus* const solid = static_cast<const G4Torus*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertGenericTrap()
{  
    const G4GenericTrap* const solid = static_cast<const G4GenericTrap*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTrap()
{  
    const G4Trap* const solid = static_cast<const G4Trap*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTrd()
{  
    const G4Trd* const solid = static_cast<const G4Trd*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTubs()
{  
    const G4Tubs* const solid = static_cast<const G4Tubs*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertCutTubs()
{  
    const G4CutTubs* const solid = static_cast<const G4CutTubs*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTwistedBox()
{  
    const G4TwistedBox* const solid = static_cast<const G4TwistedBox*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTwistedTrap()
{  
    const G4TwistedTrap* const solid = static_cast<const G4TwistedTrap*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTwistedTrd()
{  
    const G4TwistedTrd* const solid = static_cast<const G4TwistedTrd*>(m_solid);
    assert(solid); 
    assert(0) ; 
}
void X4SolidBase::convertTwistedTubs()
{  
    const G4TwistedTubs* const solid = static_cast<const G4TwistedTubs*>(m_solid);
    assert(solid); 
    assert(0) ; 
}

