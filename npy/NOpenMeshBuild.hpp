#pragma once

#include <vector>
#include <map>

struct NTriSource ; 
struct nnode ; 

#include "NOpenMeshType.hpp"

struct NOpenMeshCfg ; 

template <typename T> struct NOpenMesh ;
template <typename T> struct NOpenMeshProp ;
template <typename T> struct NOpenMeshDesc ;
template <typename T> struct NOpenMeshFind ;


struct NPY_API NOpenMeshConst
{
    static const char* TRIPATCH ; 
    static const char* HEXPATCH ; 
};

template <typename T>
struct NPY_API  NOpenMeshBuild
{
    typedef typename T::VertexHandle VH ; 
    typedef typename T::FaceHandle   FH ; 
    typedef typename T::HalfedgeHandle  HEH ; 
    typedef typename T::Point         P ; 

    NOpenMeshBuild( T& mesh, 
                    const NOpenMeshCfg*     cfg, 
                    NOpenMeshProp<T>&       prop, 
                    const NOpenMeshDesc<T>& desc, 
                    const NOpenMeshFind<T>& find
                  );

    VH add_vertex_unique(typename T::Point pt, bool& added ) ;  

    void add_face_(VH v0, VH v1, VH v2, VH v3 );
    FH   add_face_(VH v0, VH v1, VH v2, int identity=-1, int permute=0 );
    bool is_consistent_face_winding(VH v0, VH v1, VH v2);

    void add_parametric_primitive(const nnode* node, int level, int ctrl )  ;
    void euler_check(const nnode* node, int level );

    void copy_faces(const NOpenMesh<T>* other, int facemask );
    void copy_faces(const NTriSource*   other );

    void mark_faces(const nnode* other);
    void mark_face(FH fh, const nnode* other);
    std::string desc_facemask();


    void add_tripatch();
    void add_hexpatch(bool inner_only);
    void add_tetrahedron();
    void add_cube();


    T& mesh  ;

    const NOpenMeshCfg*     cfg ;
    NOpenMeshProp<T>&       prop ;
    const NOpenMeshDesc<T>& desc ;
    const NOpenMeshFind<T>& find ;
    int                     verbosity ; 

    std::map<int,int> f_inside_other_count ; 

};


 


