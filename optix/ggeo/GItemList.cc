#include "GItemList.hh"

#include <climits>
#include <iostream>
#include <fstream>
#include <ostream>   
#include <algorithm>
#include <iterator>

#include <boost/log/trivial.hpp>
#define LOG BOOST_LOG_TRIVIAL
// trace/debug/info/warning/error/fatal

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

const char* GItemList::GITEMLIST = "GItemList" ; 

unsigned int GItemList::UNSET = UINT_MAX ; 

unsigned int GItemList::getIndex(const char* key)
{
    if(key)
    {
        for(unsigned int i=0 ; i < m_list.size() ; i++) if(m_list[i].compare(key) == 0) return i ;  
    } 
    return UNSET  ; 
}


GItemList* GItemList::load(const char* idpath, const char* itemtype)
{
    GItemList* gil = new GItemList(itemtype);
    gil->load_(idpath); 
    return gil ;
}

void GItemList::load_(const char* idpath)
{
   fs::path cachedir(idpath);
   fs::path typedir(cachedir / GITEMLIST );

   if(fs::exists(typedir) && fs::is_directory(typedir))
   {
       fs::path txtpath(typedir);
       txtpath /= m_itemtype + ".txt" ; 

       if(fs::exists(txtpath) && fs::is_regular_file(txtpath))     
       {
            read_(txtpath.string().c_str());
       }
   }
}

void GItemList::read_(const char* txtpath)
{
   LOG(info) << "GItemList::read_ " << txtpath ;  

   std::ifstream ifs(txtpath);

   std::copy(std::istream_iterator<std::string>(ifs), 
             std::istream_iterator<std::string>(),
             std::back_inserter(m_list)); 

   ifs.close();
}


void GItemList::save(const char* idpath)
{
   fs::path cachedir(idpath);
   fs::path typedir(cachedir / GITEMLIST );

   if(!fs::exists(typedir))
   {
        if (fs::create_directories(typedir))
        {
            printf("GItemList::save created directory %s \n", typedir.string().c_str() );
        }
   }

   if(fs::exists(typedir) && fs::is_directory(typedir))
   {
       fs::path txtpath(typedir);
       txtpath /= m_itemtype + ".txt" ; 

       LOG(info) << "GItemList::save writing to " << txtpath.string() ;       

       std::ofstream ofs(txtpath.string());
       std::copy(m_list.begin(),m_list.end(),std::ostream_iterator<std::string>( ofs,"\n"));
       ofs.close();
   }
}

void GItemList::dump(const char* msg)
{
    LOG(info) << msg ; 
    std::copy( m_list.begin(),m_list.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
}


bool GItemList::operator()(const std::string& a_, const std::string& b_)
{
    std::map<std::string, unsigned int>::const_iterator end = m_order.end() ; 
    unsigned int ia = m_order.find(a_) == end ? UINT_MAX :  m_order[a_] ; 
    unsigned int ib = m_order.find(b_) == end ? UINT_MAX :  m_order[b_] ; 
    return ia < ib ; 
}

void GItemList::sort()
{
    if(m_order.size() == 0) return ; 
    std::stable_sort( m_list.begin(), m_list.end(), *this );
}




