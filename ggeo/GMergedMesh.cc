
#include <vector>
#include <climits>
#include <iostream>
#include <iomanip>

#include "BFile.hh"

// npy-
#include "Timer.hpp"
#include "NSensor.hpp"

// opticks-
#include "Opticks.hh"
#include "OpticksConst.hh"
#include "OpticksResource.hh"


#include "GGeo.hh"
#include "GSolid.hh"
#include "GParts.hh"
#include "GMergedMesh.hh"


#include "PLOG.hh"




bool GMergedMesh::isSkip()
{
   return m_geocode == OpticksConst::GEOCODE_SKIP ;  
}
bool GMergedMesh::isAnalytic()
{
   return m_geocode == OpticksConst::GEOCODE_ANALYTIC ;  
}
bool GMergedMesh::isTriangulated()
{
   return m_geocode == OpticksConst::GEOCODE_TRIANGULATED ;  
}



GMergedMesh::GMergedMesh(unsigned int index)
       : 
       GMesh(index, NULL, 0, NULL, 0, NULL, NULL),
       m_cur_vertices(0),
       m_cur_faces(0),
       m_cur_solid(0),
       m_num_csgskip(0),
       m_cur_base(NULL),
       m_parts(new GParts())
{
} 


std::string GMergedMesh::brief()
{
    std::stringstream ss ; 

    ss << "GMergedMesh::brief"
       << " index " << std::setw(6) << getIndex()
       << " num_csgskip " << std::setw(4) << m_num_csgskip
       << " isSkip " << std::setw(1) << isSkip()
       << " isAnalytic " << std::setw(1) << isAnalytic()
       << " isTriangulated " << std::setw(1) << isTriangulated()
       << " numVertices " << std::setw(7) << getNumVertices()
       << " numFaces " << std::setw(7) << getNumFaces()
       << " numSolids " << std::setw(5) << getNumSolids()
       << " numSolidsSelected " << std::setw(5) << getNumSolidsSelected()
       ;

    return ss.str();
}



GParts* GMergedMesh::getParts()
{
    return m_parts ; 
}
void GMergedMesh::setParts(GParts* pts) // under protest, used by the old dirty analytic PMT handling 
{
    assert(0 && "GMergedMesh::setParts who is using this ? ");
    m_parts = pts ; 
}



void GMergedMesh::setCurrentBase(GNode* base)
{
    m_cur_base = base ; 
}
GNode* GMergedMesh::getCurrentBase()
{
    return m_cur_base ; 
}


bool GMergedMesh::isGlobal()
{
    return m_cur_base == NULL ; 
}
bool GMergedMesh::isInstanced()
{
    return m_cur_base != NULL ; 
}



GMergedMesh* GMergedMesh::combine(unsigned int index, GMergedMesh* mm, GSolid* solid, unsigned verbosity )
{
    std::vector<GSolid*> solids ; 
    solids.push_back(solid);
    return combine(index, mm, solids, verbosity );
}

// count-allocate-merge
GMergedMesh* GMergedMesh::combine(unsigned int index, GMergedMesh* mm, const std::vector<GSolid*>& solids, unsigned verbosity )
{
    unsigned numSolids = solids.size(); 
    LOG(info) << "GMergedMesh::combine"
              << " making new mesh "
              << " index " << index 
              << " solids " << numSolids
              << " verbosity " << verbosity 
              ; 

    GSolid::Dump(solids, "GMergedMesh::combine (source solids)");


    GMergedMesh* com = new GMergedMesh( index ); 
    com->setVerbosity(mm ? mm->getVerbosity() : 0 );

    if(mm) com->countMergedMesh(mm, true);
    for(unsigned i=0 ; i < numSolids ; i++) com->countSolid(solids[i], true, verbosity ) ;

    com->allocate(); 
 
    if(mm) com->mergeMergedMesh(mm, true);
    for(unsigned i=0 ; i < numSolids ; i++) com->mergeSolid(solids[i], true, verbosity ) ;

    com->updateBounds();
    com->dumpSolids("GMergedMesh::combine (combined result) ");
   
    return com ; 
}



GMergedMesh* GMergedMesh::create(unsigned ridx, GNode* base, GNode* root, unsigned verbosity )
{
    // for instanced meshes the base is set to the first occurence of the 
    // instance eg invoked from GScene::makeMergedMeshAndInstancedBuffers

    assert(root && "root node is required");

    Timer t("GMergedMesh::create") ; 
    t.setVerbose(false);
    t.start();

    GMergedMesh* mm = new GMergedMesh( ridx ); 
    mm->setCurrentBase(base);  // <-- when NULL it means will use global not base relative transforms

    GNode* start = base ? base : root ; 


    if(verbosity > 1)
    LOG(info)<<"GMergedMesh::create"
             << " ridx " << ridx 
             << " starting from " << start->getName() ;
             ; 

    // 1st pass traversal : counts vertices and faces

    mm->traverse_r( start, 0, PASS_COUNT, verbosity  );  

    t("1st pass traverse");

    // allocate space for flattened arrays

    if(verbosity > 1)
    LOG(info) << mm->brief() ; 

    mm->allocate(); 

    // 2nd pass traversal : merge copy GMesh into GMergedMesh 

    mm->traverse_r( start, 0, PASS_MERGE, verbosity );  
    t("2nd pass traverse");

    mm->updateBounds();

    t("updateBounds");

    t.stop();
    //t.dump();

    return mm ;
}


// NB what is appropriate for a merged mesh is not for a mesh ... wrt counting solids
// so cannot lump the below together using GMesh base class

void GMergedMesh::countMergedMesh( GMergedMesh*  other, bool selected)
{
    unsigned int nsolid = other->getNumSolids();

    m_num_solids += nsolid ;

    if(selected)
    {
        m_num_solids_selected += 1 ;
        countMesh( other ); 
    }

    if(m_verbosity > 1)
    LOG(info) << "GMergedMesh::count other GMergedMesh  " 
              << " selected " << selected
              << " num_solids " << m_num_solids 
              << " num_solids_selected " << m_num_solids_selected 
              ;
}

void GMergedMesh::countSolid( GSolid* solid, bool selected, unsigned verbosity )
{
    const GMesh* mesh = solid->getMesh();

    m_num_solids += 1 ; 

    if(selected)
    {
        m_num_solids_selected += 1 ;
        countMesh( mesh ); 
    }

    if(m_verbosity > 1)
    LOG(info) << "GMergedMesh::count GSolid " 
              << " verbosity " << verbosity 
              << " selected " << selected
              << " num_solids " << m_num_solids 
              << " num_solids_selected " << m_num_solids_selected 
              ;
}

void GMergedMesh::countMesh( const GMesh* mesh )
{
    unsigned int nface = mesh->getNumFaces();
    unsigned int nvert = mesh->getNumVertices();
    unsigned int meshIndex = mesh->getIndex();

    m_num_vertices += nvert ;
    m_num_faces    += nface ; 
    m_mesh_usage[meshIndex] += 1 ;  // which meshes contribute to the mergedmesh
}


void GMergedMesh::mergeMergedMesh( GMergedMesh* other, bool selected )
{
    // solids are present irrespective of selection as prefer absolute solid indexing 

    //assert(0 && "is this being used ?");  // users will probably need to accomodate new way of handling analytic...
/*
    // YEP : 
    frame #4: 0x0000000100d79dcf libGGeo.dylib`GMergedMesh::mergeMergedMesh(this=0x000000010acf6200, other=0x000000010acf3390, selected=true) + 63 at GMergedMesh.cc:263
    frame #5: 0x0000000100d79836 libGGeo.dylib`GMergedMesh::combine(index=0, mm=0x000000010acf3390, solids=0x00007fff5fbfd220, verbosity=1) + 870 at GMergedMesh.cc:140
    frame #6: 0x0000000100d79468 libGGeo.dylib`GMergedMesh::combine(index=0, mm=0x000000010acf3390, solid=0x000000010acf5860, verbosity=1) + 664 at GMergedMesh.cc:115
    frame #7: 0x0000000100d60419 libGGeo.dylib`GGeoTest::createPmtInBox(this=0x000000010ac4ea50) + 1209 at GGeoTest.cc:179
    frame #8: 0x0000000100d5fb5e libGGeo.dylib`GGeoTest::create(this=0x000000010ac4ea50) + 126 at GGeoTest.cc:109
    frame #9: 0x0000000100d5fa3d libGGeo.dylib`GGeoTest::modifyGeometry(this=0x000000010ac4ea50) + 157 at GGeoTest.cc:81
    frame #10: 0x0000000100d842bc libGGeo.dylib`GGeo::modifyGeometry(this=0x0000000107b11ae0, config=0x0000000000000000) + 668 at GGeo.cc:819
    frame #11: 0x00000001010f6844 libOpticksGeometry.dylib`OpticksGeometry::modifyGeometry(this=0x0000000107b12cb0) + 868 at OpticksGeometry.cc:263
    frame #12: 0x00000001010f5d8c libOpticksGeometry.dylib`OpticksGeometry::loadGeometry(this=0x0000000107b12cb0) + 572 at OpticksGeometry.cc:200
    frame #13: 0x00000001010f9e69 libOpticksGeometry.dylib`OpticksHub::loadGeometry(this=0x00007fff5fbfeae0) + 409 at OpticksHub.cc:243
    frame #14: 0x00000001010f8ffd libOpticksGeometry.dylib`OpticksHub::init(this=0x00007fff5fbfeae0) + 77 at OpticksHub.cc:94
    frame #15: 0x00000001010f8f00 libOpticksGeometry.dylib`OpticksHub::OpticksHub(this=0x00007fff5fbfeae0, ok=0x00007fff5fbfeb50) + 416 at OpticksHub.cc:81
    frame #16: 0x00000001010f90dd libOpticksGeometry.dylib`OpticksHub::OpticksHub(this=0x00007fff5fbfeae0, ok=0x00007fff5fbfeb50) + 29 at OpticksHub.cc:83
    frame #17: 0x000000010000d026 CTestDetectorTest`main(argc=1, argv=0x00007fff5fbfee58) + 950 at CTestDetectorTest.cc:48
    frame #18: 0x00007fff8a48b5fd libdyld.dylib`start + 1
*/


    unsigned int nsolid = other->getNumSolids();

    if(m_verbosity > 1)
    LOG(info) << "GMergedMesh::mergeMergedMesh"
              << " m_cur_solid " << m_cur_solid
              << " m_cur_vertices " << m_cur_vertices
              << " m_cur_faces " << m_cur_faces
              << " other nsolid " << nsolid 
              << " selected " << selected
              ; 


    for(unsigned int i=0 ; i < nsolid ; i++)
    {
        gbbox bb = other->getBBox(i) ;
        gfloat4 ce = other->getCenterExtent(i) ;

        if(m_verbosity > 2)
        LOG(info) << "GMergedMesh::mergeMergedMesh"
                   << " m_cur_solid " << m_cur_solid  
                   << " i " << i
                   << " ce " <<  ce.description() 
                   << " bb " <<  bb.description() 
                   ;

        m_bbox[m_cur_solid] = bb ;  
        m_center_extent[m_cur_solid] = ce ;
        m_nodeinfo[m_cur_solid] = other->getNodeInfo(i) ; 
        m_identity[m_cur_solid] = other->getIdentity(i) ; 
        m_meshes[m_cur_solid] = other->getMeshIndice(i) ; 

        memcpy( getTransform(m_cur_solid), other->getTransform(i), 16*sizeof(float) ); 

        m_cur_solid += 1 ; 
    }

    unsigned int nvert = other->getNumVertices();
    unsigned int nface = other->getNumFaces();

    gfloat3* vertices = other->getVertices();
    gfloat3* normals = other->getNormals();
    guint3*  faces = other->getFaces();

    unsigned* node_indices = other->getNodes();
    unsigned* boundary_indices = other->getBoundaries();
    unsigned* sensor_indices = other->getSensors();

    if(selected)
    {
        mergeSolidVertices( nvert, vertices, normals );
        mergeSolidFaces(    nface, faces, node_indices, boundary_indices, sensor_indices );

        m_cur_vertices += nvert ;
        m_cur_faces    += nface ;
        // offsets within the flat arrays
    }
}



void GMergedMesh::mergeSolid( GSolid* solid, bool selected, unsigned verbosity )
{
    GNode* node = static_cast<GNode*>(solid);
    GNode* base = getCurrentBase();
    unsigned ridx = solid->getRepeatIndex() ;  

    GMatrixF* transform = base ? solid->getRelativeTransform(base) : solid->getTransform() ;     // base or root relative global transform

    // GMergedMesh::create invokes GMergedMesh::mergeSolid from node tree traversal 
    // via the recursive GMergedMesh::traverse_r 
    //
    // GNode::getRelativeTransform
    //     relative transform calculated from the product of ancestor transforms
    //     after the base node (ie traverse ancestors starting from root to this node
    //     but only collect transforms after the base node : which is required to be 
    //     an ancestor of this node)
    //
    // GNode::getTransform
    //     global transform, ie product of all transforms from root to this node
    //  
    //
    // When node == base the transform is identity
    //
    if( node == base ) assert( transform->isIdentity() ); 
    if( ridx == 0 ) assert( base == NULL && "expecting NULL base for ridx 0" ); 


    float* dest = getTransform(m_cur_solid);
    assert(dest);
    transform->copyTo(dest);

    const GMesh* mesh = solid->getMesh();   // triangulated
    GParts* pts = solid->getParts();  // analytic 

    unsigned num_vert = mesh->getNumVertices();
    unsigned num_face = mesh->getNumFaces();

    guint3* faces = mesh->getFaces();
    gfloat3* vertices = mesh->getTransformedVertices(*transform) ;
    gfloat3* normals  = mesh->getTransformedNormals(*transform);  

    if(verbosity > 1) mergeSolidDump(solid);
    mergeSolidBBox(vertices, num_vert);
    mergeSolidIdentity(solid, selected );

    m_cur_solid += 1 ;    // irrespective of selection, as prefer absolute solid indexing 

    if(selected)
    {
        mergeSolidVertices( num_vert, vertices, normals );

        unsigned* node_indices     = solid->getNodeIndices();
        unsigned* boundary_indices = solid->getBoundaryIndices();
        unsigned* sensor_indices   = solid->getSensorIndices();

        mergeSolidFaces( num_face, faces, node_indices, boundary_indices, sensor_indices  );

        mergeSolidAnalytic( pts, transform, verbosity );

        // offsets with the flat arrays
        m_cur_vertices += num_vert ;  
        m_cur_faces    += num_face ; 
    }
}


void GMergedMesh::mergeSolidDump( GSolid* solid)
{
    const char* pvn = solid->getPVName() ;
    const char* lvn = solid->getLVName() ;
    guint4 _identity = solid->getIdentity();
    unsigned ridx = solid->getRepeatIndex() ;  

    LOG(info) << "GMergedMesh::mergeSolidDump" 
              << " m_cur_solid " << m_cur_solid
              << " idx " << solid->getIndex()
              << " ridx " << ridx
              << " id " << _identity.description()
              << " pv " << ( pvn ? pvn : "-" )
              << " lv " << ( lvn ? lvn : "-" )
              ;
}

void GMergedMesh::mergeSolidBBox( gfloat3* vertices, unsigned nvert )
{
    // needs to be outside the selection branch for the all solid center extent
    gbbox* bb = GMesh::findBBox(vertices, nvert) ;
    if(bb == NULL) LOG(fatal) << "GMergedMesh::mergeSolid NULL bb " ; 
    assert(bb); 

    m_bbox[m_cur_solid] = *bb ;  
    m_center_extent[m_cur_solid] = bb->center_extent() ;
}

void GMergedMesh::mergeSolidIdentity( GSolid* solid, bool selected )
{
    const GMesh* mesh = solid->getMesh();

    unsigned nvert = mesh->getNumVertices();
    unsigned nface = mesh->getNumFaces();

    guint4 _identity = solid->getIdentity();

    unsigned nodeIndex = solid->getIndex();
    unsigned meshIndex = mesh->getIndex();
    unsigned boundary = solid->getBoundary();

    NSensor* sensor = solid->getSensor();
    unsigned sensorIndex = NSensor::RefIndex(sensor) ; 

    assert(_identity.x == nodeIndex);
    assert(_identity.y == meshIndex);
    assert(_identity.z == boundary);
    //assert(_identity.w == sensorIndex);   this is no longer the case, now require SensorSurface in the identity
    
    LOG(debug) << "GMergedMesh::mergeSolidIdentity"
              << " m_cur_solid " << m_cur_solid 
              << " nodeIndex " << nodeIndex
              << " boundaryIndex " << boundary
              << " sensorIndex " << sensorIndex
              << " sensor " << ( sensor ? sensor->description() : "NULL" )
              ;


    GNode* parent = solid->getParent();
    unsigned int parentIndex = parent ? parent->getIndex() : UINT_MAX ;

    m_meshes[m_cur_solid] = meshIndex ; 

    // face and vertex counts must use same selection as above to be usable 
    // with the above filled vertices and indices 

    m_nodeinfo[m_cur_solid].x = selected ? nface : 0 ; 
    m_nodeinfo[m_cur_solid].y = selected ? nvert : 0 ; 
    m_nodeinfo[m_cur_solid].z = nodeIndex ;  
    m_nodeinfo[m_cur_solid].w = parentIndex ; 

    if(isGlobal())
    {
         if(nodeIndex != m_cur_solid)
             LOG(fatal) << "GMergedMesh::mergeSolidIdentity mismatch " 
                        <<  " nodeIndex " << nodeIndex 
                        <<  " m_cur_solid " << m_cur_solid
                        ; 

         //assert(nodeIndex == m_cur_solid);  // trips ggv-pmt still needed ?
    } 
    m_identity[m_cur_solid] = _identity ; 
}

void GMergedMesh::mergeSolidVertices( unsigned nvert, gfloat3* vertices, gfloat3* normals )
{
    for(unsigned i=0 ; i < nvert ; ++i )
    {
        m_vertices[m_cur_vertices+i] = vertices[i] ; 
        m_normals[m_cur_vertices+i] = normals[i] ; 
    }
}

void GMergedMesh::mergeSolidFaces( unsigned nface, guint3* faces, unsigned* node_indices, unsigned* boundary_indices, unsigned* sensor_indices )
{
    assert(node_indices);
    assert(boundary_indices);
    assert(sensor_indices);

    for(unsigned i=0 ; i < nface ; ++i )
    {
        m_faces[m_cur_faces+i].x = faces[i].x + m_cur_vertices ;  
        m_faces[m_cur_faces+i].y = faces[i].y + m_cur_vertices ;  
        m_faces[m_cur_faces+i].z = faces[i].z + m_cur_vertices ;  

        // TODO: consolidate into uint4 
        m_nodes[m_cur_faces+i]      = node_indices[i] ;
        m_boundaries[m_cur_faces+i] = boundary_indices[i] ;
        m_sensors[m_cur_faces+i]    = sensor_indices[i] ;
    }

}


void GMergedMesh::mergeSolidAnalytic( GParts* pts, GMatrixF* transform, unsigned verbosity )
{
    // analytic CSG combined at node level  

    if(!pts)
    {
        LOG(debug) << "GMergedMesh::mergeSolidAnalytic pts NULL " ;
        return ; 
    }

    if(transform && !transform->isIdentity())
    {
        pts->applyPlacementTransform(transform, verbosity );
    }
    m_parts->add(pts, verbosity); 
}


void GMergedMesh::traverse_r( GNode* node, unsigned int depth, unsigned int pass, unsigned verbosity )
{
    GSolid* solid = dynamic_cast<GSolid*>(node) ;

    int idx = getIndex() ;
    assert(idx > -1 ) ; 

    unsigned uidx = idx > -1 ? idx : UINT_MAX ; 
    unsigned ridx = solid->getRepeatIndex() ;

    bool repsel =  idx == -1 || ridx == uidx ;
    bool csgskip = solid->isCSGSkip() ; 
    bool selected_ =  solid->isSelected() && repsel ;
    bool selected = selected_ && !csgskip ;

    if(pass == PASS_COUNT)
    {
         if(selected_ && csgskip) m_num_csgskip++ ; 
    }

    if(verbosity > 1)
          LOG(info)
                  << "GMergedMesh::traverse_r"
                  << " verbosity " << verbosity
                  << " node " << node 
                  << " solid " << solid 
                  << " solid.pts " << solid->getParts()
                  << " depth " << depth 
                  << " NumChildren " << node->getNumChildren()
                  << " pass " << pass
                  << " selected " << selected
                  << " csgskip " << csgskip
                  ; 


    switch(pass)
    {
       case PASS_COUNT:    countSolid(solid, selected, verbosity)  ;break;
       case PASS_MERGE:    mergeSolid(solid, selected, verbosity)  ;break;
               default:    assert(0)                    ;break;
    }

    for(unsigned int i = 0; i < node->getNumChildren(); i++) traverse_r(node->getChild(i), depth + 1, pass, verbosity );
}



void GMergedMesh::reportMeshUsage(GGeo* ggeo, const char* msg)
{
     LOG(info) << msg ; 
     typedef std::map<unsigned int, unsigned int>::const_iterator MUUI ; 

     unsigned int tv(0) ; 
     for(MUUI it=m_mesh_usage.begin() ; it != m_mesh_usage.end() ; it++)
     {
         unsigned int meshIndex = it->first ; 
         unsigned int nodeCount = it->second ; 
 
         GMesh* mesh = ggeo->getMesh(meshIndex);
         const char* meshName = mesh->getName() ; 
         unsigned int nv = mesh->getNumVertices() ; 
         unsigned int nf = mesh->getNumFaces() ; 

         printf("  %4d (v%5d f%5d) : %6d : %7d : %s \n", meshIndex, nv, nf, nodeCount, nodeCount*nv, meshName);

         tv += nodeCount*nv ; 
     }
     printf(" tv : %7d \n", tv);
}




GMergedMesh* GMergedMesh::load(Opticks* opticks, unsigned int ridx, const char* version)
{
    std::string mmpath = opticks->getResource()->getMergedMeshPath(ridx);
    GMergedMesh* mm = GMergedMesh::load(mmpath.c_str(), ridx, version);
    return mm ; 
}



GMergedMesh* GMergedMesh::load(const char* dir, unsigned int index, const char* version)
{
    GMergedMesh* mm(NULL);

    std::string cachedir = BFile::FormPath(dir, NULL, NULL);
    bool existsdir = BFile::ExistsDir(dir, NULL, NULL);

    LOG(info) << "GMergedMesh::load"
              << " dir " << dir 
              << " -> cachedir " << cachedir
              << " index " << index
              << " version " << version
              << " existsdir " << existsdir
              ;
 

    if(!existsdir)
    {
        LOG(warning) << "GMergedMesh::load directory DOES NOT EXIST " <<  dir ;
    }
    else
    {
        mm = new GMergedMesh(index);
        if(index == 0) mm->setVersion(version);  // mesh versioning applies to  global buffer 
        mm->loadBuffers(cachedir.c_str());
    }
    return mm ; 
}



/*
void GMergedMesh::dumpPostCache(const char* msg)
{
    LOG(info) << msg ; 
}
*/


void GMergedMesh::dumpSolids(const char* msg)
{
    gfloat4 ce0 = getCenterExtent(0) ;
    LOG(info) << msg << " ce0 " << ce0.description() ; 

    for(unsigned int index=0 ; index < getNumSolids() ; ++index)
    {
        gfloat4 ce = getCenterExtent(index) ;
        gbbox bb = getBBox(index) ; 
        std::cout 
             << std::setw(5)  << index         
             << " ce " << std::setw(64) << ce.description()       
             << " bb " << std::setw(64) << bb.description()       
             << std::endl 
             ;
    }

    for(unsigned int index=0 ; index < getNumSolids() ; ++index)
    {
        guint4* ni = getNodeInfo() + index ; 
        guint4* id = getIdentity() + index ; 
        std::cout 
             << std::setw(5)  << index         
             << " ni[nf/nv/nidx/pidx]"  << ni->description()
             << " id[nidx,midx,bidx,sidx] " << id->description() 
             << std::endl 
             ;
    }
}



float* GMergedMesh::getModelToWorldPtr(unsigned int index)
{
    return index == 0 ? GMesh::getModelToWorldPtr(0) : NULL ;
}


