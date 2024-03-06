#pragma once
/**
U4Mesh.h : Polygonize and Serialize Solids into Triangles/Quads
==================================================================

This selects and simplifies some parts of the former x4/X4Mesh.{hh,cc} 
and it better integrated with the pyvista PolyData face format
allowing 3D visualization with quads as well as triangles. 

+------------+------------------------------------------------------------------------+------------------------------------+ 
| field      |  content                                                               |  thoughts                          |
+============+========================================================================+====================================+
| (NP*)vtx   |  (num_vtx, 3) array of float coordinates                               | basis                              |       
+------------+------------------------------------------------------------------------+------------------------------------+ 
| (NP*)fpd   |  flat vtx index array : funny pyvista irregular 3/4 vertex face format | pv useful, includes quads, DROP?   |
+------------+------------------------------------------------------------------------+------------------------------------+ 
| (NP*)face  |  (num_face,4) tri/quad vtx index array using f.face[:,3] == -1 for tri | DROP ?                             |
+------------+------------------------------------------------------------------------+------------------------------------+ 
| (NP*)tri   |  (num_tri,3) tri vtx index array                                       | standard                           |
+------------+------------------------------------------------------------------------+------------------------------------+ 
| (NP*)tpd   |  flat vtx index array : funny pyvista using all triangles              | same as tri, pv useful             | 
+------------+------------------------------------------------------------------------+------------------------------------+ 

For use with OpenGL rendering its natural to use "vtx" and "tri".

**/

#include <map>
#include "G4Polyhedron.hh"
#include "NPX.h"
#include "NPFold.h"

struct U4Mesh
{
    const G4VSolid* solid ; 
    G4Polyhedron*   poly ; 
    std::map<int,int> v2fc ;  

    int             nv_poly, nv, nf ; 
    NP*             vtx ; 
    double*         vtx_ ; 
    NP*             fpd ;  // funny pyvista irregular 3/4 vertex face format

    NP*             face ;  // can include tri and quad
    int*            face_ ; 
    int             nt ; 
    NP*             tri ;  // only triangle indices (more standard gltf suitable layout than other face formats) 
    int*            tri_ ;  
    NP*             tpd ;  // funny pyvista face format, but with all 3 vertex

    static void Save(const G4VSolid* solid, const char* base="$FOLD"); 
    static NPFold* MakeFold(const std::vector<const G4VSolid*>& solids ) ; 
    static NPFold* Serialize(const G4VSolid* solid) ; 
    U4Mesh(const G4VSolid* solid);     
    void init(); 

    void init_vtx_face_count(); 
    int getVertexFaceCount(int iv) const ; 
    std::string desc_vtx_face_count() const ; 

    void init_vtx(); 
    void init_fpd();
    NPFold* serialize() const ; 
    void save(const char* base) const ; 
    std::string desc() const  ; 

    void init_face(); 
    void init_tri();
    void init_tpd();
 
};

inline void U4Mesh::Save(const G4VSolid* solid, const char* base) // static
{
    NPFold* fold = U4Mesh::Serialize(solid) ; 
    fold->save(base); 
}
inline NPFold* U4Mesh::MakeFold(const std::vector<const G4VSolid*>& solids ) // static
{
    NPFold* mesh = new NPFold ; 
    int num_solid = solids.size(); 
    for(int i=0 ; i < num_solid ; i++)
    {
        const G4VSolid* so = solids[i];
        G4String name = so->GetName() ;  // CAUTION: BY VALUE 
        const char* key = name.c_str() ; 
        NPFold* sub = Serialize(so) ;    
        mesh->add_subfold( key, sub ); 
    }
    return mesh ; 
}

inline NPFold* U4Mesh::Serialize(const G4VSolid* solid) // static
{
    //G4Polyhedron::SetNumberOfRotationSteps(24); 
    U4Mesh mesh(solid); 
    return mesh.serialize() ; 
}

inline U4Mesh::U4Mesh(const G4VSolid* solid_):
    solid(solid_),
    poly(solid->CreatePolyhedron()),
    nv_poly(poly->GetNoVertices()),
    nv(0),
    nf(poly->GetNoFacets()),
    vtx(nullptr),
    vtx_(nullptr),
    fpd(nullptr)
   ,face(NP::Make<int>(nf,4)),
    face_(face->values<int>()),
    nt(0),
    tri(nullptr),
    tri_(nullptr),
    tpd(nullptr)
{
    init(); 
}

inline void U4Mesh::init()
{
    init_vtx_face_count(); 

    init_vtx() ; 
    init_fpd() ; 

    init_face() ; 
    init_tri() ; 
    init_tpd() ; 

}

/**
U4Mesh::init_vtx_face_count
----------------------------

For each vtx count how many faces it is referenced from
and keep that in a map. This is used to 
exclude vertices that are not referenced 
by any facet in the output vtx array. 

This is needed as find that the polygonization of G4Orb 
via G4PolyhedronSphere includes a vertex (0,0,0) 
that is not referenced from any facet which causes 
issues for generation of normals, yeilding (nan,nan,nan) 
due to the attempt at smooth normalization.

**/
inline void U4Mesh::init_vtx_face_count()
{
    for(int i=0 ; i < nf ; i++)
    {
        G4int nedge;
        G4int ivertex[4];
        G4int iedgeflag[4];
        G4int ifacet[4];
        poly->GetFacet(i+1, nedge, ivertex, iedgeflag, ifacet);
        assert( nedge == 3 || nedge == 4  ); 

        for(int j=0 ; j < nedge ; j++)
        {
            G4int iv = ivertex[j] - 1 ; 
            if(v2fc.count(iv) == 0)
            {
               v2fc[iv] = 1 ; 
            }
            else
            {
                v2fc[iv] += 1 ; 
            }
        }
    }

    nv = v2fc.size(); 
    //std::cout << desc_vtx_face_count() ; 
}


/**
U4Mesh::getVertexFaceCount
---------------------------

Returns the number of faces that use the provided 0-based vertex index.
Note that the count is obtained from the original triangles and quads, 
so will not exactly match the count derived after splitting quads into triangles. 

**/

inline int U4Mesh::getVertexFaceCount(int iv) const 
{
    return v2fc.count(iv) == 1 ? v2fc.at(iv) : 0  ; // map can only have 0 or 1 occurrences of iv key 
}

inline std::string U4Mesh::desc_vtx_face_count() const 
{
    std::stringstream ss ; 
    ss << "U4Mesh::desc_vtx_face_count" << std::endl ; 
    typedef std::map<int,int> MII ; 
    for(MII::const_iterator it = v2fc.begin() ; it != v2fc.end() ; it++)
    {
        int iv = it->first ; 
        int fc = it->second ; 
        int vfc = getVertexFaceCount(iv) ; 
        assert( fc == vfc ); 
        ss << iv << ":" << fc << std::endl ; 
    }

    ss << "nv_poly:" << nv_poly << "nv:" << nv << std::endl ; 
    ss << " getVertexFaceCount(0) " << getVertexFaceCount(0) << std::endl ; 
    ss << " getVertexFaceCount(nv_poly-1) :" << getVertexFaceCount(nv_poly-1) << std::endl ; 
    ss << " getVertexFaceCount(nv-1)      :"  << getVertexFaceCount(nv-1) << std::endl ; 

    std::string str = ss.str() ;
    return str ;  
}

inline void U4Mesh::init_vtx()
{
    vtx = NP::Make<double>(nv, 3) ; 
    vtx_ = vtx->values<double>() ; 

    int nv_count = 0 ; 
    for(int iv=0 ; iv < nv_poly ; iv++)
    {
        int vfc = getVertexFaceCount(iv) ; 
        G4Point3D point = poly->GetVertex(iv+1) ;  
        if( vfc == 0 )
        {
            std::cout 
                << "U4Mesh::init_vtx"
                << " DISQUALIFIED VTX NOT INCLUDED IN ANY FACET " 
                << " iv " << iv 
                << " vfc " << vfc 
                << " point [ "
                << point.x()
                << ","
                << point.y()
                << ","
                << point.z()
                << "]" 
                << std::endl 
                ;
        }  
        else
        {
            vtx_[3*nv_count+0] = point.x() ; 
            vtx_[3*nv_count+1] = point.y() ; 
            vtx_[3*nv_count+2] = point.z() ; 
            nv_count += 1 ;  
        }
    }
    assert( nv_count == nv ); 
}

/**
U4Mesh::init_fpd
--------------------

The format needed by general pyvista PolyData faces 
(with tri and quad for example) requires an irregular array 
that is not easy to form from the existing regular face array.
But its trivial to form the array in C++, so here it is.
For example the faces of a quad and two triangles::

    faces = np.hstack([[4, 0, 1, 2, 3],[3, 0, 1, 4],[3, 1, 2, 4]]) 

Create pyvista PolyData from the fpd and vtx array with the below::

    pd = pv.PolyData(f.vtx, f.fpd)

For use of that in context see u4/tests/U4Mesh_test.py 

**/
inline void U4Mesh::init_fpd()
{
    std::vector<int> _fpd ; 
    for(int i=0 ; i < nf ; i++)
    {
        G4int nedge;
        G4int ivertex[4];
        G4int iedgeflag[4];
        G4int ifacet[4];
        poly->GetFacet(i+1, nedge, ivertex, iedgeflag, ifacet);
        assert( nedge == 3 || nedge == 4  ); 

        if( nedge == 3 )
        {
            _fpd.push_back(3);  
            _fpd.push_back(ivertex[0]-1);
            _fpd.push_back(ivertex[1]-1);
            _fpd.push_back(ivertex[2]-1);
        } 
        else if( nedge == 4 )
        {
            _fpd.push_back(4);  
            _fpd.push_back(ivertex[0]-1);
            _fpd.push_back(ivertex[1]-1);
            _fpd.push_back(ivertex[2]-1);
            _fpd.push_back(ivertex[3]-1);
        }
    }
    fpd = NPX::Make<int>(_fpd); 
}

inline NPFold* U4Mesh::serialize() const 
{
    NPFold* fold = new NPFold ; 
    fold->add("vtx",  vtx ); 
    fold->add("fpd",  fpd ); 

    fold->add("face", face ); 
    fold->add("tri",  tri ); 
    fold->add("tpd",  tpd ); 

    return fold ; 
}

inline void U4Mesh::save(const char* base) const 
{
    NPFold* fold = serialize(); 
    fold->save(base); 
}



inline std::string U4Mesh::desc() const 
{
    std::stringstream ss ; 
    ss << "U4Mesh::desc" 
       << " nv " << nv 
       << " nf " << nf 
       << std::endl 
       ; 
    std::string str = ss.str(); 
    return str ; 
}


/**
U4Mesh::init_face
-------------------

* G4Polyhedron providing 1-based vertex indices, 
  convert those to more standard 0-based 

**/

inline void U4Mesh::init_face()
{
    nt = 0 ; 
    for(int i=0 ; i < nf ; i++)
    {
        G4int nedge;
        G4int ivertex[4];
        G4int iedgeflag[4];
        G4int ifacet[4];
        poly->GetFacet(i+1, nedge, ivertex, iedgeflag, ifacet);
        assert( nedge == 3 || nedge == 4  ); 

        face_[i*4+0] = ivertex[0]-1 ;
        face_[i*4+1] = ivertex[1]-1 ;
        face_[i*4+2] = ivertex[2]-1 ;
        face_[i*4+3] = nedge == 4 ? ivertex[3]-1 : -1 ;

        switch(nedge)  // count triangles for init_tri
        {
            case 3: nt += 1 ; break ; 
            case 4: nt += 2 ; break ; 
        }
    }
}


/**
U4Mesh::init_tri
-----------------

Quads are split into two tri::

           w-------z
           |       |
           |       |
           |       |
           x-------y

           +-------z
           |     / |
           |   /   |   1:(x->y->z)   0,1,2 
           | /  (1)|
           x-------y

           w-------z
           | (2) / |
           |   /   |   2:(x->z->w)  0,2,3
           | /     |
           x-------+


HMM: this assumes repeatability of GetFacet ? 

* G4Polyhedron provides 1-based vertex indices, 
  convert those to much more standard 0-based 

Note that using tri from pyvista is not so convenient 
as have to change format to that needed by pv.PolyData::

    def PolyData_FromTRI(f):
        tri = np.zeros( (len(f.tri), 4 ), dtype=np.int32 )
        tri[:,0] = 3 
        tri[:,1:] = f.tri    
        pd = pv.PolyData(f.vtx, tri)
        return pd 

**/

inline void U4Mesh::init_tri()
{
    assert( nt > 0 ); 
    tri = NP::Make<int>(nt, 3 ); 
    tri_ = tri->values<int>() ; 
    int jt = 0 ; 

    for(int i=0 ; i < nf ; i++)
    {
        G4int nedge;
        G4int ivertex[4];
        G4int iedgeflag[4];
        G4int ifacet[4];
        poly->GetFacet(i+1, nedge, ivertex, iedgeflag, ifacet);
        assert( nedge == 3 || nedge == 4  );

        if( nedge == 3 )
        { 
            assert( jt < nt ); 
            for(int j=0 ; j < 3 ; j++) tri_[jt*3+j] = ivertex[j]-1 ;
            jt += 1 ; 
        } 
        else if( nedge == 4 )
        {
            assert( jt < nt ); 
            for(int j=0 ; j < 3 ; j++) tri_[jt*3+j] = ivertex[j]-1 ;  // x,y,z
            jt += 1 ; 

            assert( jt < nt ); 
            tri_[jt*3+0] = ivertex[0]-1 ; // x 
            tri_[jt*3+1] = ivertex[2]-1 ; // z 
            tri_[jt*3+2] = ivertex[3]-1 ; // w 
            jt += 1 ; 
        }
    }
    assert( nt == jt ); 
}

inline void U4Mesh::init_tpd()
{
    std::vector<int> _tpd ; 
    for(int i=0 ; i < nf ; i++)
    {
        G4int nedge;
        G4int ivertex[4];
        G4int iedgeflag[4];
        G4int ifacet[4];
        poly->GetFacet(i+1, nedge, ivertex, iedgeflag, ifacet);
        assert( nedge == 3 || nedge == 4  );

        if( nedge == 3 )
        { 
            _tpd.push_back(3) ; 
            for(int j=0 ; j < 3 ; j++) _tpd.push_back(ivertex[j]-1); 
        } 
        else if( nedge == 4 )
        {
            _tpd.push_back(3) ; 
            for(int j=0 ; j < 3 ; j++) _tpd.push_back(ivertex[j]-1) ; // x,y,z

            _tpd.push_back(3) ; 
            _tpd.push_back(ivertex[0]-1);  // x
            _tpd.push_back(ivertex[2]-1);  // z
            _tpd.push_back(ivertex[3]-1);  // w
        }
    }
    tpd = NPX::Make<int>(_tpd); 
}


