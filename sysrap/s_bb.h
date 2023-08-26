#pragma once
/**
s_bb.h
========

*/

#include <string>
#include <sstream>
#include <iomanip>

#include "s_pool.h"

struct _s_bb
{
    static constexpr const char* ITEM = "6" ;  

    double x0 ; 
    double y0 ; 
    double z0 ; 
    double x1 ; 
    double y1 ; 
    double z1 ; 
};

#include "SYSRAP_API_EXPORT.hh"

struct SYSRAP_API s_bb
{
    static constexpr const int N = 6 ; 
    static constexpr const char* NAME = "s_bb.npy" ; 
    static constexpr const bool LEAK = false ; 
    typedef s_pool<s_bb,_s_bb> POOL ;
    static POOL* pool ;
    static void SetPOOL( POOL* pool_ ); 
    static int level() ; 
    static bool IsZero( const double* v ); 
    static void Serialize( _s_bb& p, const s_bb* o ); 
    static s_bb* Import(  const _s_bb* p, const std::vector<_s_bb>& buf ); 
    
    s_bb(); 
    ~s_bb(); 

    int pid ; 

    double x0 ; 
    double y0 ; 
    double z0 ; 
    double x1 ; 
    double y1 ; 
    double z1 ; 


    const double* cdata() const { return &x0 ; }
    double* data() {              return &x0 ; }

    bool is_root() const { return true ; } 
    std::string desc() const ;  

    static void IncludePoint( double* aabb,  const double* other_point ); 
    static void IncludeAABB(  double* aabb,  const double* other_aabb  ); 
    void include_point( const double* point ); 
    void include_aabb(  const double* aabb  ); 

}; 

inline void s_bb::SetPOOL( POOL* pool_ ){ pool = pool_ ; }
inline int s_bb::level() {  return pool ? pool->level : ssys::getenvint("sn__level",-1) ; } // static 

inline bool s_bb::IsZero( const double* v )
{
    int count = 0 ; 
    for(int i=0 ; i < N ; i++) if(v[i] == 0.) count += 1 ; 
    return count == N ; 
}




inline void s_bb::Serialize( _s_bb& p, const s_bb* o )
{
    p.x0 = o->x0 ; 
    p.y0 = o->y0 ; 
    p.z0 = o->z0 ; 
    p.x1 = o->x1 ; 
    p.y1 = o->y1 ; 
    p.z1 = o->z1 ; 
} 
inline s_bb* s_bb::Import( const _s_bb* p, const std::vector<_s_bb>& )
{
    s_bb* o = new s_bb ; 
    o->x0 = p->x0 ; 
    o->y0 = p->y0 ; 
    o->z0 = p->z0 ; 
    o->x1 = p->x1 ; 
    o->y1 = p->y1 ; 
    o->z1 = p->z1 ; 
    return o ; 
}


inline s_bb::s_bb()
    :
    pid(pool ? pool->add(this) : -1),
    x0(0.),
    y0(0.),
    z0(0.),
    x1(0.),
    y1(0.),
    z1(0.)
{
    if(level() > 1) std::cerr << "s_bb::s_bb pid " << pid << std::endl ; 
}
inline s_bb::~s_bb()
{
    if(level() > 1) std::cerr << "s_bb::~s_bb pid " << pid << std::endl ; 
    if(pool) pool->remove(this); 
}


inline std::string s_bb::desc() const 
{
    int w = 7 ; 
    int p = 3 ;  
    std::stringstream ss ;
    ss 
       << "[" << std::setw(w) << std::fixed << std::setprecision(p) << x0
       << "," << std::setw(w) << std::fixed << std::setprecision(p) << y0
       << "," << std::setw(w) << std::fixed << std::setprecision(p) << z0 
       << "," << std::setw(w) << std::fixed << std::setprecision(p) << x1
       << "," << std::setw(w) << std::fixed << std::setprecision(p) << y1 
       << "," << std::setw(w) << std::fixed << std::setprecision(p) << z1 
       << "]" 
       ;
        
    std::string str = ss.str(); 
    return str ; 
}


inline void s_bb::IncludePoint( double* aabb,  const double* point ) // static
{
    double* x0 = aabb + 0 ; 
    double* y0 = aabb + 1 ; 
    double* z0 = aabb + 2 ; 

    double* x1 = aabb + 3 ; 
    double* y1 = aabb + 4 ; 
    double* z1 = aabb + 5 ; 

    *x0 = std::min( *x0 , point[0] ) ; 
    *y0 = std::min( *y0 , point[1] ) ; 
    *z0 = std::min( *z0 , point[2] ) ; 

    *x1 = std::max( *x1 , point[0] ) ; 
    *y1 = std::max( *y1 , point[1] ) ; 
    *z1 = std::max( *z1 , point[2] ) ; 
}

inline void s_bb::IncludeAABB(  double* aabb,  const double* other_aabb  ) // static
{
    double* x0 = aabb + 0 ; 
    double* y0 = aabb + 1 ; 
    double* z0 = aabb + 2 ; 

    double* x1 = aabb + 3 ; 
    double* y1 = aabb + 4 ; 
    double* z1 = aabb + 5 ; 

    *x0 = std::min( *x0 , other_aabb[0] ) ; 
    *y0 = std::min( *y0 , other_aabb[1] ) ; 
    *z0 = std::min( *z0 , other_aabb[2] ) ; 

    *x1 = std::max( *x1 , other_aabb[3] ) ; 
    *y1 = std::max( *y1 , other_aabb[4] ) ; 
    *z1 = std::max( *z1 , other_aabb[5] ) ; 

}
inline void s_bb::include_point( const double* point )
{
    IncludePoint( data(), point ); 
}
inline void s_bb::include_aabb(  const double* aabb  )
{
    IncludeAABB( data(), aabb ); 
}





