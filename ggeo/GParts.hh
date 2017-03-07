#pragma once

#include <map>
#include <string>
#include <vector>

#include <glm/fwd.hpp>

struct npart ; 
struct NSlice ; 
template <typename T> class NPY ;

struct guint4 ; 
struct gbbox ; 
struct gfloat3 ; 

class GItemList ; 
class GBndLib ; 

/**

GParts
======= 

Creates *primitive* buffer (formerly called *solid*) from the *parts* buffer
the *parts* buffer .npy for DYB PMT geometry is created by detdesc partitioning with pmt-/tree.py 
OR for test geometries it is created part-by-part using methods of the npy primitive structs, see eg::

   npy/NPart.hpp
   npy/NSphere.hpp 

**/


#include "GGEO_API_EXPORT.hh"
#include "GGEO_HEAD.hh"

class GGEO_API GParts {
    public:
       // conventional names for interfacing
       static const char* CONTAINING_MATERIAL ; 
       static const char* SENSOR_SURFACE ; 
    public:
       // shapes with analytic intersection implementations 
       static const char* SPHERE_ ;
       static const char* TUBS_ ; 
       static const char* BOX_ ; 
       static const char* PRISM_ ; 
       static const char* INTERSECTION_; 
       static const char* UNION_ ; 
       static const char* DIFFERENCE_ ; 
       //static const char* TypeName(unsigned int typecode);

    public:
        // buffer layout, must match locations in pmt-/tree.py:convert 
        enum { 
              QUADS_PER_ITEM = 4, 
              NJ = 4,
              NK = 4,
              SK = 4  
            } ;

    public:
        static GParts* make(const npart& pt, const char* spec);
        static GParts* make(char typecode, glm::vec4& param, const char* spec);
    public:
        static GParts* combine(std::vector<GParts*> subs);
    public:
        GParts(GBndLib* bndlib=NULL);
        GParts(NPY<float>* buffer, const char* spec, GBndLib* bndlib=NULL);
        GParts(NPY<float>* buffer, GItemList* spec, GBndLib* bndlib=NULL);
    public:
        void setName(const char* name);
        void setBndLib(GBndLib* blib);
        void setVerbose(bool verbose); 
        void add(GParts* other);
        void close();
        bool isClosed();
        void enlargeBBox(unsigned int part, float epsilon=0.00001f);
        void enlargeBBoxAll(float epsilon=0.00001f);
    private:
        void init(const char* spec);        
        void init();        
    public: 
        const char*  getName();
        unsigned int getIndex(unsigned int part);
        unsigned int getFlags(unsigned int part);
        unsigned int getTypeCode(unsigned int part);
        unsigned int getNodeIndex(unsigned int part);
        unsigned int getBoundary(unsigned int part);
    public: 
        std::string  getBoundaryName(unsigned int part);
        const char*  getTypeName(unsigned int part);
   private:
        gbbox        getBBox(unsigned int i);
        gfloat3      getGfloat3(unsigned int i, unsigned int j, unsigned int k);
        float*       getValues(unsigned int i, unsigned int j, unsigned int k);
    public:
        guint4       getPrimInfo(unsigned int iprim);
   public:
        void setIndex(unsigned int part, unsigned int index);
        void setFlags(unsigned int part, unsigned int flags);
        void setTypeCode(unsigned int part, unsigned int typecode);
        void setNodeIndex(unsigned int part, unsigned int nodeindex);
        void setBoundary(unsigned int part, unsigned int boundary);
        void setBoundaryAll(unsigned int boundary);
        void setFlagsAll(unsigned int flags);
    public:
        GBndLib*           getBndLib();
        GItemList*         getBndSpec();
        NPY<unsigned int>* getPrimBuffer();
        NPY<float>*        getPartBuffer();
        unsigned int       getNumPrim();
        unsigned int       getNumParts();
        unsigned int       getPrimNumParts(unsigned int prim_index);
    public:
        void dump(const char* msg="GPmt::dump");
        void dumpPrimInfo(const char* msg="GPmt::dumpPrimInfo");
        void dumpPrimBuffer(const char* msg="GPmt::dumpPrimBuffer");
        void Summary(const char* msg="GPmt::Summary");
    private:
        void dumpPrim(unsigned primIdx);
    public:
        void setSensorSurface(const char* surface="lvPmtHemiCathodeSensorSurface");
        void setContainingMaterial(const char* material="MineralOil");
        void save(const char* dir);
    private:
        void registerBoundaries();
        void makePrimBuffer();
    private:
       void         setBndSpec(GItemList* bndspec);
       void         setPartBuffer(NPY<float>* part_buffer);
       void         setPrimBuffer(NPY<unsigned int>* prim_buffer);
       unsigned int getUInt(unsigned int part, unsigned int j, unsigned int k);
       void         setUInt(unsigned int part, unsigned int j, unsigned int k, unsigned int value);
    private:
        // almost no state other than buffers, just icing on top of them
        // allowing this to copied/used on GPU in cu/hemi-pmt.cu
        NPY<float>*        m_part_buffer ; 
        GItemList*         m_bndspec ;  
        GBndLib*           m_bndlib ; 
        const char*        m_name ;         
    private:
        NPY<unsigned int>* m_prim_buffer ; 
        bool               m_closed ; 
        std::map<unsigned int, unsigned int> m_parts_per_prim ;
        std::map<unsigned int, unsigned int> m_flag_prim ;
        bool               m_verbose ; 
};

#include "GGEO_TAIL.hh"


