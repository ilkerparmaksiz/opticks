#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string/replace.hpp>


#include "BFile.hh"
#include "BOpticksEvent.hh"

#include "PLOG.hh"

const char* BOpticksEvent::DEFAULT_DIR_TEMPLATE = "$OPTICKS_EVENT_BASE/evt/$1/$2/$3" ;  // formerly "$LOCAL_BASE/env/opticks/$1/$2"
const char* BOpticksEvent::OVERRIDE_EVENT_BASE = NULL ; 

const int BOpticksEvent::DEFAULT_LAYOUT_VERSION = 1 ; 
int BOpticksEvent::LAYOUT_VERSION = 1 ; 


void BOpticksEvent::SetOverrideEventBase(const char* override_event_base)
{
   OVERRIDE_EVENT_BASE = override_event_base ? strdup(override_event_base) : NULL ; 
}
void BOpticksEvent::SetLayoutVersion(int version)
{
    LAYOUT_VERSION = version ; 
}
void BOpticksEvent::SetLayoutVersionDefault()
{
    LAYOUT_VERSION = DEFAULT_LAYOUT_VERSION ; 
}



void BOpticksEvent::Summary(const char* msg)
{
    LOG(info) << msg ; 
}

std::string BOpticksEvent::directory_template()
{
    std::string deftmpl(DEFAULT_DIR_TEMPLATE) ; 
    if(OVERRIDE_EVENT_BASE)
    {
       LOG(info) << "BOpticksEvent::directory_template OVERRIDE_EVENT_BASE replacing OPTICKS_EVENT_BASE with " << OVERRIDE_EVENT_BASE ; 
       boost::replace_first(deftmpl, "$OPTICKS_EVENT_BASE/evt", OVERRIDE_EVENT_BASE );
    } 
    return deftmpl ; 
}

std::string BOpticksEvent::directory_(const char* top, const char* sub, const char* tag)
{
    // top (geometry)
    //     old and new: BoxInBox,PmtInBox,dayabay,prism,reflect,juno,... 
    //
    // sub 
    //     old: cerenkov,oxcerenkov,oxtorch,txtorch   (constituent+source)
    //     new: cerenkov,scintillation,natural,torch  (source only)
    //
    // tag
    //     old: tag did not contribute to directory 
    //  

    std::string base = directory_template();
    boost::replace_first(base, "$1", top ); 
    boost::replace_first(base, "$2", sub ); 
    boost::replace_first(base, "$3", tag ); 
    std::string dir = BFile::FormPath( base.c_str() ); 
    return dir ; 
}
std::string BOpticksEvent::directory(const char* top, const char* sub, const char* tag)
{
    std::string dir_ = directory_(top, sub, tag);
    std::string dir = BFile::FormPath( dir_.c_str() ); 
    return dir ; 
}


std::string BOpticksEvent::path_(const char* top, const char* sub, const char* tag, const char* stem, const char* ext)
{
    std::string dir_ = directory_(top, sub, tag);
    std::stringstream ss ; 
    ss << dir_ << "/" << stem << ext ;
    std::string path = ss.str();
    return path ; 
}



std::string BOpticksEvent::path(const char* top, const char* sub, const char* tag, const char* stem, const char* ext)
{
    std::string p_ ; 
    if(LAYOUT_VERSION == 1)
    {
        // to work with 3-arg form for gensteps:  ("cerenkov","1","dayabay" )  top=dayabay sub=cerenkov tag=1 stem="" 

        char stem_source[64];
        snprintf(stem_source, 64, "%s%s", stem, sub ); 
        p_ = path_(top, stem_source, ".", tag, ext );   
    }  
    else if(LAYOUT_VERSION == 2)
    {
        const char* ustem = ( stem != NULL && stem[0] == '\0' ) ? "gs" : stem ;     
        // spring "gs" stem into life for argument compatibility with old layout : 
        // gensteps effectively has empty stem in old layout 
        p_ = path_(top, sub, tag, ustem, ext );
    }
    std::string p = BFile::FormPath( p_.c_str() ); 

     
    if(strchr(top,'%') != NULL || strchr(sub,'%') != NULL || strchr(tag,'%') != NULL  || strchr(stem,'%') != NULL || strchr(ext,'%') != NULL ) 
    {    
        LOG(fatal) << "BOpticksEvent::path OLDFORM ARGS  " 
                  << " top " << top
                  << " sub " << sub
                  << " tag " << tag 
                  << " stem " <<  stem
                  << " ext " << ext  
                  << " -> " << p 
                  ;    
    }    

    return p ; 
}


std::string BOpticksEvent::path(const char* dir, const char* name)
{
    char path[256];
    snprintf(path, 256, "%s/%s", dir, name);
    return path ; 
}




/*
std::string BOpticksEvent::directory(const char* tfmt, const char* targ, const char* det)
{

    LOG(fatal) << "BOpticksEvent::directory"
               << " tfmt " << tfmt 
               << " targ " << targ
               << " det  " << det
               ;

    char constituent_source[64];
    if(strchr (tfmt, '%' ) == NULL)
    {
        snprintf(constituent_source, 64, "%s%s", tfmt, targ ); 
    }
    else
    { 
        snprintf(constituent_source, 64, tfmt, targ ); 
    }

    std::string dir = directory(constituent_source, det);
    return dir ; 
}
std::string BOpticksEvent::path(const char* constituent, const char* source, const char* tag, const char* det)
{

// :param constituent: constituent stem, eg ox, rx, ps
// :param source: photon source eg torch, cerenkov, scintillation
// :param tag: event tag, usually numerical 
// :param det: detector tag, eg dyb, juno

    std::stringstream ss ;
    ss << constituent << source ;
    std::string constituent_source = ss.str() ;
    return path(constituent_source.c_str(), tag, det);
}

  
std::string BOpticksEvent::path(const char* constituent_source, const char* tag, const char* det)
{
// :param constituent_source: object type name, eg oxcerenkov rxcerenkov 
// :param tag: event tag, usually numerical 
// :param det: detector tag, eg dyb, juno

    //assert(0 && "move to 4-arg form");

    std::string dir = directory(constituent_source, det);
    dir += "/%s.npy" ; 

    char* tmpl = (char*)dir.c_str();
    char path_[256];
    snprintf(path_, 256, tmpl, tag );

    LOG(debug) << "BOpticksEvent::path"
              << " constituent_source " << constituent_source
              << " tag " << tag
              << " det " << det
              << " DEFAULT_DIR_TEMPLATE " << DEFAULT_DIR_TEMPLATE
              << " OVERRIDE_EVENT_BASE " << ( OVERRIDE_EVENT_BASE ? OVERRIDE_EVENT_BASE : "NULL" )
              << " tmpl " << tmpl
              << " path_ " << path_
              ;

    return path_ ;   
}

*/


