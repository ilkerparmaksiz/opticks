#pragma once

#include "stdlib.h"
#include "string.h"
#include <string>
#include <map>
#include <vector>

#include "NSequence.hpp"

class Index : public NSequence {
   public:
        typedef std::vector<std::string> VS ;
   public:
        Index(const char* itemtype, const char* title=NULL);
   public:
        static Index* load(const char* idpath, const char* itemtype);
        void save(const char* idpath);
        const char* getItemType();
   public:
        const char* getTitle();
        void setTitle(const char* title);
   public:
        int* getSelectedPtr();
        int  getSelected();
   public:
        // fulfil NSequence, in order to use with GAttrSequence
        unsigned int getNumKeys();
        const char* getKey(unsigned int i);
        unsigned int getIndex(const char* key);
   private:
        void loadMaps(const char* idpath);
        void crossreference();
   public:
        void add(const VS& vs);
        void add(const char* name, unsigned int source, bool sort=true);
        void sortNames(); // currently by ascending local index : ie addition order
        std::vector<std::string>& getNames();
        bool operator() (const std::string& a, const std::string& b);
   public:
        std::string getPrefixedString(const char* tail);
        void setExt(const char* ext);
        unsigned int getIndexLocal(const char* name, unsigned int missing=0);
        unsigned int getIndexSource(const char* name, unsigned int missing=0);
        bool         hasItem(const char* key);
        const char* getNameLocal(unsigned int local, const char* missing=NULL);
        const char* getNameSource(unsigned int source, const char* missing=NULL);

        unsigned int convertLocalToSource(unsigned int local, unsigned int missing=0);
        unsigned int convertSourceToLocal(unsigned int source, unsigned int missing=0);

   public:
        unsigned int getNumItems();
        void test(const char* msg="GItemIndex::test", bool verbose=true);
        void dump(const char* msg="GItemIndex::dump");

   private:
        const char*                          m_itemtype ; 
        const char*                          m_title ; 
        const char*                          m_ext ; 
        int                                  m_selected ; 
        std::map<std::string, unsigned int>  m_source ; 
        std::map<std::string, unsigned int>  m_local ; 
        std::map<unsigned int, unsigned int> m_source2local ; 
        std::map<unsigned int, unsigned int> m_local2source ; 
        std::vector<std::string>             m_names ; 
   private:
        // populated by formTable
        std::vector<std::string>             m_labels ; 
        std::vector<unsigned int>            m_codes ; 
};

inline Index::Index(const char* itemtype, const char* title)
   : 
   NSequence(),
   m_itemtype(strdup(itemtype)),
   m_title(title ? strdup(title) : strdup(itemtype)),
   m_ext(".json"),
   m_selected(0)
{
}
inline const char* Index::getItemType()
{
    return m_itemtype ; 
}

inline const char* Index::getTitle()
{
    return m_title ; 
}

inline void Index::setTitle(const char* title)
{
    if(!title) return ;
    free((void*)m_title);
    m_title = strdup(title); 
}


inline std::vector<std::string>& Index::getNames()
{
    return m_names ;  
}

inline void Index::setExt(const char* ext)
{   
    m_ext = strdup(ext);
}


inline int Index::getSelected()
{
    return m_selected ; 
}
inline int* Index::getSelectedPtr()
{
    return &m_selected ; 
}






