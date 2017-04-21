#include <map>
#include <iomanip>
#include <cstdio>
#include <cassert>
#include <climits>


#include "OpticksCSG.h"

// npy-
#include "NGLM.hpp"
#include "NPY.hpp"
#include "NSlice.hpp"
#include "NPart.hpp"
#include "NCSG.hpp"
#include "NQuad.hpp"
#include "NNode.hpp"
#include "GLMFormat.hpp"

#include "GVector.hh"
#include "GItemList.hh"
#include "GBndLib.hh"
#include "GParts.hh"

#include "PLOG.hh"


const char* GParts::CONTAINING_MATERIAL = "CONTAINING_MATERIAL" ;  
const char* GParts::SENSOR_SURFACE = "SENSOR_SURFACE" ;  


GParts* GParts::combine(std::vector<GParts*> subs)
{
    // Concatenate vector of GParts instances into a single GParts instance
    LOG(fatal) << "GParts::combine " << subs.size() ; 

    GParts* parts = new GParts(); 

    GBndLib* bndlib = NULL ; 
    unsigned analytic_version = 0 ;  
    OpticksCSG_t primflag = CSG_ZERO ; 


    for(unsigned int i=0 ; i < subs.size() ; i++)
    {
        GParts* sp = subs[i];

        OpticksCSG_t pf = sp->getPrimFlag();


        if(primflag == CSG_ZERO) 
            primflag = pf ; 
        else
            assert(pf == primflag && "GParts::combine requires all GParts instances to have the same primFlag " );

        unsigned av = sp->getAnalyticVersion();
        //LOG(info) << std::setw(3) << i << " " << "av " << av ; 

        if(analytic_version == 0)
            analytic_version = av ;
        else
            assert(av == analytic_version && "GParts::combine requires all GParts instances to have the same analytic_version " );   


        parts->add(sp);

        if(!bndlib) bndlib = sp->getBndLib(); 
    } 

    if(bndlib) parts->setBndLib(bndlib);
    parts->setAnalyticVersion(analytic_version);
    parts->setPrimFlag(primflag);

    return parts ; 
}


GParts* GParts::make(const npart& pt, const char* spec)
{
    // Serialize the npart shape (BOX, SPHERE or PRISM) into a (1,4,4) parts buffer. 
    // Then instanciate a GParts instance to hold the parts buffer 
    // together with the boundary spec.

    NPY<float>* partBuf = NPY<float>::make(1, NJ, NK );
    partBuf->zero();

    NPY<float>* tranBuf = NPY<float>::make(0, NTRAN, 4, 4 );
    tranBuf->zero();

    partBuf->setPart( pt, 0u );

    GParts* gpt = new GParts(partBuf,tranBuf, spec) ;

    unsigned typecode = gpt->getTypeCode(0u);
    assert(typecode == CSG_BOX || typecode == CSG_SPHERE || typecode == CSG_PRISM);

    return gpt ; 
}

GParts* GParts::make(OpticksCSG_t csgflag, glm::vec4& param, const char* spec)
{
    float size = param.w ;  
    gbbox bb(gfloat3(-size), gfloat3(size));  

    if(csgflag == CSG_ZSPHERE)
    {
        assert( 0 && "TODO: geometry specifics should live in nzsphere etc.. not here " );
        bb.min.z = param.x*param.w ; 
        bb.max.z = param.y*param.w ; 
    } 

    NPY<float>* partBuf = NPY<float>::make(1, NJ, NK );
    partBuf->zero();

    NPY<float>* tranBuf = NPY<float>::make(0, NTRAN, 4, 4 );
    tranBuf->zero();

    assert(BBMIN_K == 0 );
    assert(BBMAX_K == 0 );

    unsigned int i = 0u ; 
    partBuf->setQuad( i, PARAM_J, param.x, param.y, param.z, param.w );
    partBuf->setQuad( i, BBMIN_J, bb.min.x, bb.min.y, bb.min.z , 0.f );
    partBuf->setQuad( i, BBMAX_J, bb.max.x, bb.max.y, bb.max.z , 0.f );

    // TODO: go via an npart instance

    GParts* pt = new GParts(partBuf, tranBuf,  spec) ;
    pt->setTypeCode(0u, csgflag);

    return pt ; 
} 

const int GParts::NTRAN = 3 ; 


GParts* GParts::make( NCSG* tree)
{
    const char* spec = tree->getBoundary();
    NPY<float>* nodebuf = tree->getNodeBuffer();       // serialized binary tree
    NPY<float>* tranbuf = tree->getGTransformBuffer();  // formerly was incorrectly using TransformBuffer

    if(!tranbuf) 
    {
       tranbuf = NPY<float>::make(0,NTRAN,4,4) ;
       tranbuf->zero();
    } 
    assert( tranbuf && tranbuf->hasShape(-1,NTRAN,4,4));


    nnode* root = tree->getRoot(); 
    // hmm maybe should not use the nnode ? ie operate fully from the persistable buffers ?

    assert(nodebuf && root) ; 

    unsigned ni = nodebuf->getShape(0);
    assert( nodebuf->hasItemShape(NJ, NK) && ni > 0 );


    assert(root && root->type < CSG_UNDEFINED );

    LOG(fatal) << "GParts::make NCSG "
              << " treedir " << tree->getTreeDir()
              << " node_sh " << nodebuf->getShapeString()
              << " tran_sh " << tranbuf->getShapeString() 
              << " spec " << spec 
              << " type " << root->csgname()
              ; 

    // GParts originally intended to handle lists of parts each of which 
    // must have an associated boundary spec. When holding CSG trees there 
    // is really only a need for a single common boundary, but for
    // now enable reuse of the old GParts by duplicating the spec 
    // for every node of the tree

    GItemList* lspec = GItemList::Repeat("GParts", spec, ni ) ; 

    GParts* pts = new GParts(nodebuf, tranbuf, lspec) ;

    //pts->setTypeCode(0u, root->type);   //no need, slot 0 is the root node where the type came from
    return pts ; 
}

GParts::GParts(GBndLib* bndlib) 
      :
      m_part_buffer(NULL),
      m_tran_buffer(NULL),
      m_bndspec(NULL),
      m_bndlib(bndlib),
      m_name(NULL),
      m_prim_buffer(NULL),
      m_closed(false),
      m_verbose(false),
      m_analytic_version(0),
      m_primflag(CSG_FLAGNODETREE)
{
      init() ; 
}
GParts::GParts(NPY<float>* partBuf,  NPY<float>* tranBuf, const char* spec, GBndLib* bndlib) 
      :
      m_part_buffer(partBuf),
      m_tran_buffer(tranBuf),
      m_bndspec(NULL),
      m_bndlib(bndlib),
      m_prim_buffer(NULL),
      m_closed(false),
      m_analytic_version(0),
      m_primflag(CSG_FLAGNODETREE)
{
      init(spec) ; 
}
GParts::GParts(NPY<float>* partBuf,  NPY<float>* tranBuf, GItemList* spec, GBndLib* bndlib) 
      :
      m_part_buffer(partBuf),
      m_tran_buffer(tranBuf),
      m_bndspec(spec),
      m_bndlib(bndlib),
      m_prim_buffer(NULL),
      m_closed(false),
      m_analytic_version(0),
      m_primflag(CSG_FLAGNODETREE)
{
      init() ; 
}

void GParts::setPrimFlag(OpticksCSG_t primflag)
{
    assert(primflag == CSG_FLAGNODETREE || primflag == CSG_FLAGPARTLIST );
    m_primflag = primflag ; 
}

OpticksCSG_t GParts::getPrimFlag()
{
    return m_primflag ;
}


void GParts::save(const char* dir)
{
    if(!dir) return ; 

    const char* name = getName();    
    if(!name) name = "GParts" ; 

    LOG(info) << "GParts::save " << name << " to " << dir ; 

    if(m_part_buffer) m_part_buffer->save(dir, name, "partBuffer.npy");     
    if(m_prim_buffer) m_prim_buffer->save(dir, name, "primBuffer.npy");    
    if(m_tran_buffer) m_tran_buffer->save(dir, name, "tranBuffer.npy");    
}

void GParts::setName(const char* name)
{
    m_name = name ? strdup(name) : NULL  ; 
}
const char* GParts::getName()
{
    return m_name ; 
}

void GParts::setVerbose(bool verbose)
{
    m_verbose = verbose ; 
}


unsigned GParts::getAnalyticVersion()
{
    return m_analytic_version ; 
}
void GParts::setAnalyticVersion(unsigned version)
{
    m_analytic_version = version ; 
}
 



bool GParts::isClosed()
{
    return m_closed ; 
}

unsigned int GParts::getPrimNumParts(unsigned int prim_index)
{
    return m_parts_per_prim.count(prim_index)==1 ? m_parts_per_prim[prim_index] : 0 ; 
}


void GParts::setBndSpec(GItemList* bndspec)
{
    m_bndspec = bndspec ;
}
GItemList* GParts::getBndSpec()
{
    return m_bndspec ; 
}
void GParts::setBndLib(GBndLib* bndlib)
{
    m_bndlib = bndlib ; 
}
GBndLib* GParts::getBndLib()
{
    return m_bndlib ; 
}



void GParts::setPrimBuffer(NPY<unsigned int>* buf )
{
    m_prim_buffer = buf ; 
}
void GParts::setPartBuffer(NPY<float>* buf )
{
    m_part_buffer = buf ; 
}
void GParts::setTranBuffer(NPY<float>* buf)
{
    m_tran_buffer = buf ; 
}


NPY<unsigned int>* GParts::getPrimBuffer()
{
    return m_prim_buffer ; 
}
NPY<float>* GParts::getPartBuffer()
{
    return m_part_buffer ; 
}
NPY<float>* GParts::getTranBuffer()
{
    return m_tran_buffer ; 
}



void GParts::init(const char* spec)
{
    m_bndspec = new GItemList("GParts");
    m_bndspec->add(spec);
    init();
}

void GParts::init()
{
    if(m_part_buffer == NULL && m_tran_buffer == NULL && m_bndspec == NULL)
    {
        LOG(trace) << "GParts::init creating empty part_buffer, tran_buffer and bndspec " ; 

        m_part_buffer = NPY<float>::make(0, NJ, NK );
        m_part_buffer->zero();

        m_tran_buffer = NPY<float>::make(0, NTRAN, 4, 4 );
        m_tran_buffer->zero();

        m_bndspec = new GItemList("GParts");
    } 

    unsigned npart = m_part_buffer->getNumItems() ;
    unsigned nspec = m_bndspec->getNumItems() ;

    bool match = npart == nspec ; 
    if(!match) 
    LOG(fatal) << "GParts::init"
               << " parts/spec MISMATCH "  
               << " npart " << npart 
               << " nspec " << nspec
               ;

    assert(match);

}

unsigned int GParts::getNumParts()
{
    if(!m_part_buffer)
    {
        LOG(error) << "GParts::getNumParts NULL part_buffer" ; 
        return 0 ; 
    }

    assert(m_part_buffer->getNumItems() == m_bndspec->getNumItems() );
    return m_part_buffer->getNumItems() ;
}

void GParts::add(GParts* other)
{
    unsigned int n0 = getNumParts(); // before adding

    m_bndspec->add(other->getBndSpec());

    NPY<float>* other_part_buffer = other->getPartBuffer() ;
    NPY<float>* other_tran_buffer = other->getTranBuffer() ;

    m_part_buffer->add(other_part_buffer);
    m_tran_buffer->add(other_tran_buffer);

    unsigned num_part_add = other_part_buffer->getNumItems() ;
    unsigned num_tran_add = other_tran_buffer->getNumItems() ;

    m_part_per_add.push_back(num_part_add); 
    m_tran_per_add.push_back(num_tran_add);

    unsigned int n1 = getNumParts(); // after adding

    for(unsigned int p=n0 ; p < n1 ; p++)  // update indices for parts added
    {
        setIndex(p, p);
    }

    LOG(info) << "GParts::add"
              << " n0 " << n0  
              << " n1 " << n1
              << " num_part_add " << num_part_add
              << " num_tran_add " << num_tran_add
              << " other_part_buffer  " << other_part_buffer->getShapeString()
              << " other_tran_buffer  " << other_tran_buffer->getShapeString()
              ;  
}

void GParts::setContainingMaterial(const char* material)
{
    // for flexibility persisted GParts should leave the outer containing material
    // set to a default marker name, to allow the GParts to be placed within other geometry

    m_bndspec->replaceField(0, GParts::CONTAINING_MATERIAL, material );
}

void GParts::setSensorSurface(const char* surface)
{
    m_bndspec->replaceField(1, GParts::SENSOR_SURFACE, surface ) ; 
    m_bndspec->replaceField(2, GParts::SENSOR_SURFACE, surface ) ; 
}


void GParts::close()
{
    registerBoundaries();
    makePrimBuffer(); 
    dumpPrimBuffer(); 
}

void GParts::registerBoundaries()
{
   assert(m_bndlib); 
   unsigned int nbnd = m_bndspec->getNumKeys() ; 
   assert( getNumParts() == nbnd );
   for(unsigned int i=0 ; i < nbnd ; i++)
   {
       const char* spec = m_bndspec->getKey(i);
       unsigned int boundary = m_bndlib->addBoundary(spec);
       setBoundary(i, boundary);

       if(m_verbose)
       LOG(info) << "GParts::registerBoundaries " 
                << std::setw(3) << i 
                << " " << std::setw(30) << spec
                << " --> "
                << std::setw(4) << boundary 
                << " " << std::setw(30) << m_bndlib->shortname(boundary)
                ;

   } 
}


void GParts::makePrimBuffer()
{
    // Derives prim buffer from the parts buffer
    //
    // "prim" here was previously named "solid"
    // but thats confusing due to other solids so renamed
    // to "prim" as this corresponds to OptiX primitives GPU side, 
    // see oxrap/cu/hemi-pmt.cu::intersect
    //
    // "parts" are associated to their containing "prim" 
    // via the NodeIndex property  
    // so to prep a boolean composite need to:
    //
    // * arrange for constituent parts to share the same NodeIndex 
    // * set intersect/union/difference flag in parts buffer for first part
    //

    m_parts_per_prim.clear();

    unsigned int nmin(INT_MAX) ; 
    unsigned int nmax(0) ; 

    unsigned numParts = getNumParts() ; 

    LOG(info) << "GParts::makePrimBuffer"
              << " numParts " << numParts 
              ;
 

    // count parts for each nodeindex
    for(unsigned int i=0; i < numParts ; i++)
    {
        unsigned int nodeIndex = getNodeIndex(i);
        unsigned typ = getTypeCode(i);
        std::string  typName = CSGName((OpticksCSG_t)typ);
 
        LOG(info) << "GParts::makePrimBuffer"
                   << " i " << std::setw(3) << i  
                   << " nodeIndex " << std::setw(3) << nodeIndex
                   << " typ " << std::setw(3) << typ 
                   << " typName " << typName 
                   ;  
                     
        m_parts_per_prim[nodeIndex] += 1 ; 

        if(nodeIndex < nmin) nmin = nodeIndex ; 
        if(nodeIndex > nmax) nmax = nodeIndex ; 
    }


    // NB the number of prim does not match the number of additions ...
    //
    //    perhaps this should be arranged to simplify this ?
    //
    //    without this difficult to define a primitive transformOffset to 
    //    handle transforms from more than one pot 
    //

    LOG(info) << "GParts::makePrimBuffer"
              << " nodeIndex.min " << nmin 
              << " nodeIndex.max " << nmax
              << " parts_per_prim.size " << m_parts_per_prim.size()
              << " part_per_add.size " << m_part_per_add.size()
              << " tran_per_add.size " << m_tran_per_add.size()
              ; 
 
    unsigned int num_prim = m_parts_per_prim.size() ;

    assert( m_part_per_add.size() == num_prim );
    assert( m_tran_per_add.size() == num_prim );



    //assert(nmax - nmin == num_solids - 1);  // expect contiguous node indices
    if(nmax - nmin != num_prim - 1)
    {
        LOG(warning) << "GParts::makePrimBuffer non-contiguous node indices"
                     << " nmin " << nmin 
                     << " nmax " << nmax
                     << " num_prim " << num_prim
                     << " part_per_add.size " << m_part_per_add.size()
                     << " tran_per_add.size " << m_tran_per_add.size()
                     ; 
    }

    
    


    guint4* priminfo = new guint4[num_prim] ;

    typedef std::map<unsigned int, unsigned int> UU ; 
    unsigned part_offset = 0 ; 
    unsigned tran_offset = 0 ; 
    unsigned n = 0 ; 

    //for(UU::const_iterator it=m_parts_per_prim.begin() ; it != m_parts_per_prim.end() ; it++)

    for(unsigned i=0 ; i < num_prim ; i++)
    {
        //unsigned int node_index = it->first ; 
        //unsigned int parts_for_prim = it->second ; 
        
        unsigned int tran_for_prim = m_tran_per_add[i] ; 
        unsigned int parts_for_prim = m_parts_per_prim[i] ; 


        guint4& pri = *(priminfo+n) ;

        pri.x = part_offset ; 
        pri.y = parts_for_prim ;
        pri.z = tran_offset ; 
        pri.w = m_primflag ; 

        LOG(info) << "GParts::makePrimBuffer priminfo " << pri.description() ;       

        part_offset += parts_for_prim ; 
        tran_offset += tran_for_prim ; 

        n++ ; 
    }

    NPY<unsigned int>* buf = NPY<unsigned int>::make( num_prim, 4 );
    buf->setData((unsigned int*)priminfo);
    delete [] priminfo ; 

    setPrimBuffer(buf);
}



void GParts::dumpPrim(unsigned primIdx)
{
    // following access pattern of oxrap/cu/hemi-pmt.cu::intersect

    NPY<unsigned int>* primBuffer = getPrimBuffer();
    NPY<float>*        partBuffer = getPartBuffer();

    if(!primBuffer) return ; 
    if(!partBuffer) return ; 

    glm::uvec4 prim = primBuffer->getQuadU(primIdx) ;

    unsigned partOffset = prim.x ; 
    unsigned numParts   = prim.y ; 
    unsigned primFlag   = prim.w ; 

    LOG(info) << " primIdx "    << std::setw(3) << primIdx 
              << " partOffset " << std::setw(3) << partOffset 
              << " numParts "   << std::setw(3) << numParts
              << " primFlag "   << std::setw(5) << primFlag 
              << " CSGName "  << CSGName((OpticksCSG_t)primFlag) 
              << " prim "       << gformat(prim)
              ;

    for(unsigned int p=0 ; p < numParts ; p++)
    {
        unsigned int partIdx = partOffset + p ;

        nquad q0, q1, q2, q3 ;

        q0.f = partBuffer->getVQuad(partIdx,0);  
        q1.f = partBuffer->getVQuad(partIdx,1);  
        q2.f = partBuffer->getVQuad(partIdx,2);  
        q3.f = partBuffer->getVQuad(partIdx,3);  

        unsigned typecode = q2.u.w ;
        assert(TYPECODE_J == 2 && TYPECODE_K == 3);

        LOG(info) << " p " << std::setw(3) << p 
                  << " partIdx " << std::setw(3) << partIdx
                  << " typecode " << typecode
                  << " CSGName " << CSGName((OpticksCSG_t)typecode)
                  ;

    }


}


void GParts::dumpPrimBuffer(const char* msg)
{
    NPY<unsigned int>* primBuffer = getPrimBuffer();
    NPY<float>*        partBuffer = getPartBuffer();
    LOG(info) << msg ; 
    if(!primBuffer) return ; 
    if(!partBuffer) return ; 

    LOG(info) 
        << " primBuffer " << primBuffer->getShapeString() 
        << " partBuffer " << partBuffer->getShapeString() 
        ; 

    assert( primBuffer->hasItemShape(4,0) && primBuffer->getNumItems() > 0  );
    assert( partBuffer->hasItemShape(4,4) && partBuffer->getNumItems() > 0 );

    for(unsigned primIdx=0 ; primIdx < primBuffer->getNumItems() ; primIdx++) dumpPrim(primIdx);
}


void GParts::dumpPrimInfo(const char* msg)
{
    unsigned int numPrim = getNumPrim() ;
    LOG(info) << msg << " (part_offset, parts_for_prim, prim_index, prim_flags) numPrim:" << numPrim  ;

    for(unsigned int i=0 ; i < numPrim ; i++)
    {
        guint4 pri = getPrimInfo(i);
        LOG(info) << pri.description() ;
    }
}


void GParts::Summary(const char* msg)
{
    LOG(info) << msg 
              << " num_parts " << getNumParts() 
              << " num_prim " << getNumPrim()
              ;
 
    typedef std::map<unsigned int, unsigned int> UU ; 
    for(UU::const_iterator it=m_parts_per_prim.begin() ; it!=m_parts_per_prim.end() ; it++)
    {
        unsigned int prim_index = it->first ; 
        unsigned int nparts = it->second ; 
        unsigned int nparts2 = getPrimNumParts(prim_index) ; 
        printf("%2u : %2u \n", prim_index, nparts );
        assert( nparts == nparts2 );
    }

    for(unsigned int i=0 ; i < getNumParts() ; i++)
    {
        std::string bn = getBoundaryName(i);
        printf(" part %2u : node %2u type %2u boundary [%3u] %s  \n", i, getNodeIndex(i), getTypeCode(i), getBoundary(i), bn.c_str() ); 
    }
}




unsigned int GParts::getNumPrim()
{
    return m_prim_buffer ? m_prim_buffer->getShape(0) : 0 ; 
}
const char* GParts::getTypeName(unsigned int part_index)
{
    unsigned int code = getTypeCode(part_index);
    return CSGName((OpticksCSG_t)code);
}
     
float* GParts::getValues(unsigned int i, unsigned int j, unsigned int k)
{
    float* data = m_part_buffer->getValues();
    float* ptr = data + i*NJ*NK+j*NJ+k ;
    return ptr ; 
}
     
gfloat3 GParts::getGfloat3(unsigned int i, unsigned int j, unsigned int k)
{
    float* ptr = getValues(i,j,k);
    return gfloat3( *ptr, *(ptr+1), *(ptr+2) ); 
}
guint4 GParts::getPrimInfo(unsigned int iprim)
{
    unsigned int* data = m_prim_buffer->getValues();
    unsigned int* ptr = data + iprim*SK  ;
    return guint4( *ptr, *(ptr+1), *(ptr+2), *(ptr+3) );
}
gbbox GParts::getBBox(unsigned int i)
{
   gfloat3 min = getGfloat3(i, BBMIN_J, BBMIN_K );  
   gfloat3 max = getGfloat3(i, BBMAX_J, BBMAX_K );  
   gbbox bb(min, max) ; 
   return bb ; 
}

void GParts::enlargeBBoxAll(float epsilon)
{
   for(unsigned int part=0 ; part < getNumParts() ; part++) enlargeBBox(part, epsilon);
}

void GParts::enlargeBBox(unsigned int part, float epsilon)
{
    float* pmin = getValues(part,BBMIN_J,BBMIN_K);
    float* pmax = getValues(part,BBMAX_J,BBMAX_K);

    glm::vec3 min = glm::make_vec3(pmin) - glm::vec3(epsilon);
    glm::vec3 max = glm::make_vec3(pmax) + glm::vec3(epsilon);
 
    *(pmin+0 ) = min.x ; 
    *(pmin+1 ) = min.y ; 
    *(pmin+2 ) = min.z ; 

    *(pmax+0 ) = max.x ; 
    *(pmax+1 ) = max.y ; 
    *(pmax+2 ) = max.z ; 

    LOG(debug) << "GParts::enlargeBBox"
              << " part " << part 
              << " epsilon " << epsilon
              << " min " << gformat(min) 
              << " max " << gformat(max)
              ; 

}


unsigned int GParts::getUInt(unsigned int i, unsigned int j, unsigned int k)
{
    assert(i < getNumParts() );
    unsigned int l=0u ; 
    return m_part_buffer->getUInt(i,j,k,l);
}
void GParts::setUInt(unsigned int i, unsigned int j, unsigned int k, unsigned int value)
{
    assert(i < getNumParts() );
    unsigned int l=0u ; 
    m_part_buffer->setUInt(i,j,k,l, value);
}



unsigned int GParts::getNodeIndex(unsigned int part)
{
    return getUInt(part, NODEINDEX_J, NODEINDEX_K);
}
unsigned int GParts::getTypeCode(unsigned int part)
{
    return getUInt(part, TYPECODE_J, TYPECODE_K);
}
unsigned int GParts::getIndex(unsigned int part)
{
    return getUInt(part, INDEX_J, INDEX_K);
}
unsigned int GParts::getBoundary(unsigned int part)
{
    return getUInt(part, BOUNDARY_J, BOUNDARY_K);
}




void GParts::setNodeIndex(unsigned int part, unsigned int nodeindex)
{
    setUInt(part, NODEINDEX_J, NODEINDEX_K, nodeindex);
}
void GParts::setTypeCode(unsigned int part, unsigned int typecode)
{
    setUInt(part, TYPECODE_J, TYPECODE_K, typecode);
}
void GParts::setIndex(unsigned int part, unsigned int index)
{
    setUInt(part, INDEX_J, INDEX_K, index);
}
void GParts::setBoundary(unsigned int part, unsigned int boundary)
{
    setUInt(part, BOUNDARY_J, BOUNDARY_K, boundary);
}





void GParts::setBoundaryAll(unsigned int boundary)
{
    for(unsigned int i=0 ; i < getNumParts() ; i++) setBoundary(i, boundary);
}
void GParts::setNodeIndexAll(unsigned int nodeindex)
{
    for(unsigned int i=0 ; i < getNumParts() ; i++) setNodeIndex(i, nodeindex);
}


std::string GParts::getBoundaryName(unsigned int part)
{
    unsigned int boundary = getBoundary(part);
    std::string name = m_bndlib ? m_bndlib->shortname(boundary) : "" ;
    return name ;
}



void GParts::fulldump(const char* msg)
{
    LOG(info) << msg ; 

    dump(msg);
    Summary(msg);

    NPY<float>* partBuf = getPartBuffer();
    NPY<unsigned int>* primBuf = getPrimBuffer(); 

    partBuf->dump("partBuf");
    primBuf->dump("primBuf:partOffset/numParts/primIndex/0");
}

void GParts::dump(const char* msg)
{
    LOG(info) << "GParts::dump " << msg ; 

    dumpPrimInfo(msg);

    NPY<float>* buf = m_part_buffer ; 
    assert(buf);
    assert(buf->getDimensions() == 3);

    unsigned int ni = buf->getShape(0) ;
    unsigned int nj = buf->getShape(1) ;
    unsigned int nk = buf->getShape(2) ;

    LOG(info) << "GParts::dump ni " << ni ; 

    assert( nj == NJ );
    assert( nk == NK );

    float* data = buf->getValues();

    uif_t uif ; 

    for(unsigned int i=0; i < ni; i++)
    {   
       unsigned int tc = getTypeCode(i);
       unsigned int id = getIndex(i);
       unsigned int bnd = getBoundary(i);
       std::string  bn = getBoundaryName(i);

       std::string csg = CSGName((OpticksCSG_t)tc);

       const char*  tn = getTypeName(i);

       for(unsigned int j=0 ; j < NJ ; j++)
       {   
          for(unsigned int k=0 ; k < NK ; k++) 
          {   
              uif.f = data[i*NJ*NK+j*NJ+k] ;
              if( j == TYPECODE_J && k == TYPECODE_K )
              {
                  assert( uif.u == tc );
                  printf(" %10u (%s) TYPECODE ", uif.u, tn );
              } 
              else if( j == INDEX_J && k == INDEX_K)
              {
                  assert( uif.u == id );
                  printf(" %6u <-INDEX   " , uif.u );
              }
              else if( j == BOUNDARY_J && k == BOUNDARY_K)
              {
                  assert( uif.u == bnd );
                  printf(" %6u <-bnd  ", uif.u );
              }
              else if( j == NODEINDEX_J && k == NODEINDEX_K)
                  printf(" %10d (nodeIndex) ", uif.i );
              else
                  printf(" %10.4f ", uif.f );
          }   

          if( j == BOUNDARY_J ) printf(" bn %s ", bn.c_str() );
          printf("\n");
       }   
       printf("\n");
    }   
}


