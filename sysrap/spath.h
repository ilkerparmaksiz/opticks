#pragma once
/**
spath.h
=========

Q: Whats the difference between spath::ResolvePath and spath::Resolve ? 
A: ResolvePath accepts only a single string element whereas Resolve accepts
   from 1 to 4 elements. Also ResolvePath is private, the public interface is Resolve 

**/

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

#include "sproc.h"

struct spath
{
    friend struct spath_test ; 
    static constexpr const bool VERBOSE = false ; 
    static constexpr const bool DUMP = false ; 

private:
    static std::string _ResolvePath(const char* spec); 
    static std::string _ResolvePathGeneralized(const char* spec_); 

    static char* ResolvePath(const char* spec); 
    static char* ResolvePathGeneralized(const char* spec); 

    static char* DefaultTMP();
    static char* DefaultOutputDir();
    static constexpr const char* _DefaultOutputDir = "$TMP/GEOM/$GEOM/$ExecutableName" ; 

    static char* ResolveToken(const char* token); 
    static char* _ResolveToken(const char* token); 
    static bool  IsTokenWithFallback(const char* token); 
    static bool  IsToken(const char* token); 
    static char* _ResolveTokenWithFallback(const char* token); 

    template<typename ... Args>
    static std::string _Resolve(Args ... args ); 

public:
    template<typename ... Args>
    static const char* Resolve(Args ... args ); 

private:
    template<typename ... Args>
    static std::string _Join( Args ... args_  ); 

    template<typename ... Args>
    static std::string _Name( Args ... args_  ); 

public:
    template<typename ... Args>
    static const char* Join( Args ... args ); 

    template<typename ... Args>
    static const char* Name( Args ... args ); 


    template<typename ... Args>
    static bool Exists( Args ... args ); 

    static bool LooksLikePath(const char* arg); 
    static const char* Basename(const char* path); 

    static int Remove(const char* path_); 

    static const char* SearchDirUpTreeWithFile( const char* startdir, const char* relf ); 

};


/**
spath::_ResolvePath
----------------------

This works with multiple tokens, eg::

    $HOME/.opticks/GEOM/$GEOM/CSGFoundry/meshname.txt

But not yet with fallback paths, see _ResolvePathGeneralized

**/

inline std::string spath::_ResolvePath(const char* spec_)
{
    if(spec_ == nullptr) return "" ; 
    char* spec = strdup(spec_); 

    std::stringstream ss ; 
    int speclen = int(strlen(spec)) ;  
    char* end = strchr(spec, '\0' ); 
    int i = 0 ; 

    if(VERBOSE) std::cout << " spec " << spec << " speclen " << speclen << std::endl ; 

    while( i < speclen )
    {
        if(VERBOSE) std::cout << " i " << i << " spec[i] " << spec[i] << std::endl ;   
        if( spec[i] == '$' )
        {
            char* p = spec + i ; 
            char* sep = strchr( p, '/' ) ; // first slash after $ : HMM too simple with fallback paths
            bool tok_plus =  sep && end && sep != end ;  
            if(tok_plus) *sep = '\0' ;           // replace slash with null termination 
            char* val = ResolveToken(p+1) ;  // skip '$'
            int toklen = int(strlen(p)) ;  // strlen("TOKEN")  no need for +1 as already at '$'  
            if(VERBOSE) std::cout << " toklen " << toklen << std::endl ;  
            if(val == nullptr) 
            {
                std::cerr 
                    << "spath::_ResolvePath token [" 
                    << p+1 
                    << "] does not resolve " 
                    << std::endl 
                    ; 
                ss << "UNRESOLVED_TOKEN_" << (p+1) ; 
            }
            else
            {
                ss << val ; 
            }
            if(tok_plus) *sep = '/' ;  // put back the slash 
            i += toklen ;              // skip over the token 
        }
        else
        {
           ss << spec[i] ; 
           i += 1 ; 
        }
    }
    std::string str = ss.str(); 
    return str ; 
}

/**
spath::_ResolvePathGeneralized
---------------------------------

Simple paths with tokens::

    $HOME/.opticks/GEOM/$GEOM/CSGFoundry/meshname.txt

More generalized paths with curlies and fallback paths::

    ${GEOM}Hello 
    ${RNGDir:-$HOME/.opticks/rngcache/RNG}

**/

inline std::string spath::_ResolvePathGeneralized(const char* spec_)
{
    if(spec_ == nullptr) return "" ; 
    char* spec = strdup(spec_); 

    std::stringstream ss ; 
    int speclen = int(strlen(spec)) ;  
    int speclen1 = speclen - 1 ; 
    char* end = strchr(spec, '\0' ); 
    int i = 0 ; 

    while( i < speclen  )
    {
        char* p = spec + i ; 
        if( i < speclen1 && *p == '$' && *(p+1) != '{' )
        {
            char* sep = strchr( p, '/' ) ;       // first slash after $ : HMM too simple with fallback paths
            bool tok_plus =  sep && end && sep != end ;  
            if(tok_plus) *sep = '\0' ;           // replace slash with null termination 
            char* val = ResolveToken(p+1) ;      // skip '$'
            int toklen = int(strlen(p)) ;        // strlen("TOKEN")  no need for +1 as already at '$'  
            ss << ( val ? val : p+1 )  ; 
            if(tok_plus) *sep = '/' ;            // put back the slash 
            i += toklen ;                        // skip over the token 
        }
        else if( i < speclen1 && *p == '$' && *(p+1) == '{' )
        {
            char* sep = strchr(p, '}' ) + 1 ;   // one char beyond the first } after the $ 
            char keep = *sep ; 
            bool tok_plus = sep && end && sep != end ;  
            if(tok_plus) *sep = '\0' ;           // replace one beyond char with null termination 
            char* val = ResolveToken(p+1) ;      // skip '$'
            int toklen = int(strlen(p)) ;        // strlen("TOKEN")  no need for +1 as already at '$'  
            ss << ( val ? val : p+1 )  ; 
            if(tok_plus) *sep = keep ;            // put back the changed char (could be '\0' ) 
            i += toklen ;                        // skip over the token 
        }
        else
        {
           ss << *p ; 
           i += 1 ; 
        }
    }
    std::string str = ss.str(); 
    return str ; 
}




inline char* spath::ResolvePath(const char* spec_)
{
    std::string path = _ResolvePath(spec_) ;
    return strdup(path.c_str()) ; 
}
inline char* spath::ResolvePathGeneralized(const char* spec_)
{
    std::string path = _ResolvePathGeneralized(spec_) ;
    return strdup(path.c_str()) ; 
}




inline char* spath::DefaultTMP()
{
    char* user = getenv("USER") ; 
    std::stringstream ss ; 
    ss << "/tmp/" << ( user ? user : "MISSING_USER" ) << "/" << "opticks" ;   
    std::string str = ss.str(); 
    return strdup(str.c_str()); 
}

inline char* spath::DefaultOutputDir()
{
    return (char*)_DefaultOutputDir ; 
}



inline char* spath::ResolveToken(const char* token)
{
    bool is_twf = IsTokenWithFallback(token) ; 

    char* result0 = is_twf 
                 ?
                    _ResolveTokenWithFallback(token)
                 :
                    _ResolveToken(token)
                 ; 
    
    bool is_still_token = IsToken(result0) && strcmp(token, result0) != 0 ; 
    char* result1 = is_still_token ? ResolvePath(result0) : result0  ;  

    if(DUMP) std::cout 
        << "spath::ResolveToken.DUMP" << std::endl 
        << " token  [" << ( token ? token : "-" ) << "]"
        << " is_twf " << ( is_twf ? "YES" : "NO " ) 
        << " result0 [" << ( result0 ? result0 : "-" ) << "]"
        << " result1 [" << ( result1 ? result1 : "-" ) << "]"
        << std::endl 
        ;
      
    return result1 ; 
}

/**
spath::_ResolveToken
----------------------

The token "TMP" is special cased to resolve as /tmp/$USER/opticks
when no TMP envvar is defined. 
Examples of accepted tokens::

    TMP
    $TMP
    ${VERSION:-0}

**/


inline char* spath::_ResolveToken(const char* token)
{
    char* tok = strdup(token) ; 

    if( tok && strlen(tok) > 0 && tok[0] == '$') tok += 1 ;  // advance past leading '$'
    if( tok && strlen(tok) > 0 && tok[0] == '{' && tok[strlen(tok)-1] == '}') // trim leading and trailing { }
    {
        tok += 1 ; 
        tok[strlen(tok)-1] = '\0' ; 
    } 

    char* val = getenv(tok) ; 
    if( val == nullptr && strcmp(tok, "TMP") == 0)              val = DefaultTMP() ; 
    if( val == nullptr && strcmp(tok, "ExecutableName")   == 0) val = sproc::ExecutableName() ; 
    if( val == nullptr && strcmp(tok, "DefaultOutputDir") == 0) val = DefaultOutputDir() ; 
    return val ; 
}

/**
spath::IsTokenWithFallback
---------------------------

Bash style token with fallback::

   ${U4Debug_SaveDir:-$TMP}   # original 
   {U4Debug_SaveDir:-$TMP}    # when arrives here from ResolveToken 


**/

inline bool spath::IsTokenWithFallback(const char* token)
{
    return token && strlen(token) > 0 && token[0] == '{' && token[strlen(token)-1] == '}' && strstr(token,":-") != nullptr  ; 
}
inline bool spath::IsToken(const char* token)
{
    return token && strlen(token) > 0 && strstr(token,"$") != nullptr  ; 
}


/**
spath::_ResolveTokenWithFallback
----------------------------------

Currently only simple fallback token are supported::

    ${FIRST:-$SECOND}

TODO handle::
 
    ${VERSION:-0}

**/

inline char* spath::_ResolveTokenWithFallback(const char* token_)
{
    char* token = strdup(token_); 
    if(token && strlen(token) > 0 && token[0] == '$') token += 1 ; 

    bool is_twf = IsTokenWithFallback(token) ;
    if(!is_twf) std::cerr << "spath::_ResolveTokenWithFallback"
         << " ERROR NOT-IsTokenWithFallback " 
         << " token_ [" << ( token_ ? token_ : "-" ) << "]" 
         << " token [" << ( token ? token : "-" ) << "]" 
         << std::endl 
         ;       
    assert(is_twf); 

    token[strlen(token)-1] = '\0' ;  // overwrite the trailing '}' 

    const char* delim = ":-" ; 
    char* split = strstr(token, delim) ; 

    bool dump = false ; 
 
    if(dump) std::cout 
       << "spath::ResolveTokenWithFallback" 
       << std::endl 
       << " token " << ( token ? token : "-" )
       << std::endl 
       << " split " << ( split ? split : "-" )
       << std::endl 
       ;

    assert( split );  
    char* tok1 = split + strlen(delim)  ; 

    split[0] = '\0' ; 
    char* tok0 = token + 1 ; 
    
    if(dump) std::cout 
       << "spath::ResolveTokenWithFallback" 
       << std::endl 
       << " tok0 " << ( tok0 ? tok0 : "-" )
       << std::endl 
       << " tok1 " << ( tok1 ? tok1 : "-" )
       << std::endl 
       ;

    char* val = _ResolveToken(tok0) ; 
    if(val == nullptr) val = ResolvePath(tok1) ; 
    return val ; 
}




template<typename ... Args>
inline std::string spath::_Resolve( Args ... args  )  // static
{
    std::string spec = _Join(std::forward<Args>(args)... ); 
    return _ResolvePath(spec.c_str()); 
}

template std::string spath::_Resolve( const char* ); 
template std::string spath::_Resolve( const char*, const char* ); 
template std::string spath::_Resolve( const char*, const char*, const char* ); 
template std::string spath::_Resolve( const char*, const char*, const char*, const char* ); 


/**
spath::Resolve
----------------

All provided path elements must be non-nullptr
otherwise get an assert when trying to convert the 
nullptr into a std::string. 

Although this could be avoided by ignoring such 
elements it is safer to require everything defined.
Safety is important as the returned paths can be used
for directory deletions. 

**/

template<typename ... Args>
inline const char* spath::Resolve( Args ... args  )  // static
{
    std::string spec = _Join(std::forward<Args>(args)... ); 

    //std::string path = _ResolvePath(spec.c_str()); 
    std::string path = _ResolvePathGeneralized(spec.c_str()); 

    return strdup(path.c_str()) ; 
}

template const char* spath::Resolve( const char* ); 
template const char* spath::Resolve( const char*, const char* ); 
template const char* spath::Resolve( const char*, const char*, const char* ); 
template const char* spath::Resolve( const char*, const char*, const char*, const char* ); 

template<typename ... Args>
inline std::string spath::_Join( Args ... args_  )  // static
{
    std::vector<std::string> args = {args_...};
    std::vector<std::string> elem ; 

    for(unsigned i=0 ; i < args.size() ; i++)
    {
        const std::string& arg = args[i] ; 
        if(!arg.empty()) elem.push_back(arg);  
    }

    unsigned num_elem = elem.size() ; 
    std::stringstream ss ; 
    for(unsigned i=0 ; i < num_elem ; i++)
    {
        const std::string& ele = elem[i] ; 
        ss << ele << ( i < num_elem - 1 ? "/" : "" ) ; 
    }
    std::string s = ss.str(); 
    return s ; 
}   

template std::string spath::_Join( const char* ); 
template std::string spath::_Join( const char*, const char* ); 
template std::string spath::_Join( const char*, const char*, const char* ); 
template std::string spath::_Join( const char*, const char*, const char*, const char* ); 

template<typename ... Args>
inline const char* spath::Join( Args ... args )  // static
{
    std::string s = _Join(std::forward<Args>(args)...)  ; 
    return strdup(s.c_str()) ; 
}   

template const char* spath::Join( const char* ); 
template const char* spath::Join( const char*, const char* ); 
template const char* spath::Join( const char*, const char*, const char* ); 
template const char* spath::Join( const char*, const char*, const char*, const char* ); 








template<typename ... Args>
inline std::string spath::_Name( Args ... args_  )  // static
{
    std::vector<std::string> args = {args_...};
    std::vector<std::string> elem ; 

    for(unsigned i=0 ; i < args.size() ; i++)
    {
        const std::string& arg = args[i] ; 
        if(!arg.empty()) elem.push_back(arg);  
    }

    const char* delim = "" ; 

    unsigned num_elem = elem.size() ; 
    std::stringstream ss ; 
    for(unsigned i=0 ; i < num_elem ; i++)
    {
        const std::string& ele = elem[i] ; 
        ss << ele << ( i < num_elem - 1 ? delim : "" ) ; 
    }
    std::string s = ss.str(); 
    return s ; 
}   

template std::string spath::_Name( const char* ); 
template std::string spath::_Name( const char*, const char* ); 
template std::string spath::_Name( const char*, const char*, const char* ); 
template std::string spath::_Name( const char*, const char*, const char*, const char* ); 

template<typename ... Args>
inline const char* spath::Name( Args ... args )  // static
{
    std::string s = _Name(std::forward<Args>(args)...)  ; 
    return strdup(s.c_str()) ; 
}   

template const char* spath::Name( const char* ); 
template const char* spath::Name( const char*, const char* ); 
template const char* spath::Name( const char*, const char*, const char* ); 
template const char* spath::Name( const char*, const char*, const char*, const char* ); 





















template<typename ... Args>
inline bool spath::Exists(Args ... args)
{
    std::string path = _Resolve(std::forward<Args>(args)...) ; 
    std::ifstream fp(path.c_str(), std::ios::in|std::ios::binary);
    return fp.fail() ? false : true ; 
}

template bool spath::Exists( const char* ); 
template bool spath::Exists( const char*, const char* ); 
template bool spath::Exists( const char*, const char*, const char* ); 
template bool spath::Exists( const char*, const char*, const char*, const char* ); 


inline bool spath::LooksLikePath(const char* arg)
{
    if(!arg) return false ;
    if(strlen(arg) < 2) return false ; 
    return arg[0] == '/' || arg[0] == '$' ; 
}

inline const char* spath::Basename(const char* path)
{
    std::string p = path ; 
    std::size_t pos = p.find_last_of("/");
    std::string base = pos == std::string::npos ? p : p.substr(pos+1) ; 
    return strdup( base.c_str() ) ; 
}


inline int spath::Remove(const char* path_)
{
    const char* path = spath::Resolve(path_); 
    assert( strlen(path) > 2 ); 
    return remove(path);  
}


/**
spath::SearchDirUpTreeWithFile
-------------------------------

Search up the directory tree starting from *startdir* for 
a directory that contains an existing relative filepath *relf*  

**/

inline const char* spath::SearchDirUpTreeWithFile( const char* startdir, const char* relf )
{
    if(startdir == nullptr || relf == nullptr) return nullptr ; 
    char* dir = strdup(startdir) ; 
    while(dir && strlen(dir) > 1)
    {
        if(spath::Exists(dir, relf)) break ; 
        char* last = strrchr(dir, '/');    
        *last = '\0' ;  // move the null termination inwards from right, going up directory by directory 
    }
    return ( dir && strlen(dir) > 1 ) ? strdup(dir) : nullptr ; 
}

