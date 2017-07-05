#include "PLOG.hh"

#include "OpticksCSG.h"
#include "OpticksCSGMask.h"

#include "NGLMExt.hpp"
#include "GLMFormat.hpp"
#include "NNode.hpp"
#include "Nuv.hpp"
#include "NBBox.hpp"
#include "NNodeUncoincide.hpp"

#include "NPrimitives.hpp"

NNodeUncoincide::NNodeUncoincide(nnode* node, unsigned verbosity)
   :
   m_node(node),
   m_verbosity(verbosity)
{
}

unsigned NNodeUncoincide::uncoincide()
{
    // canonically invoked via nnode::uncoincide from NCSG::import_r

    nnode* a = NULL ; 
    nnode* b = NULL ; 

    unsigned rc = 0 ; 

    if(m_node->is_root())
    {
        rc = uncoincide_treewise();
    }

    // NB BELOW PAIRWISE APPROACH CURRENTLY NOT USED
    else if(is_uncoincidable_subtraction(a,b))
    {
        rc = uncoincide_subtraction(a,b);
    } 
    else if(is_uncoincidable_union(a,b))
    {
        rc = uncoincide_union(a,b);
    }

    return rc ; 
}


bool NNodeUncoincide::is_uncoincidable_subtraction(nnode*& a, nnode*& b)
{
    assert(!m_node->complement); 
    // hmm theres an implicit assumption here that all complements have 
    // already fed down to the leaves

    OpticksCSG_t type = m_node->type ; 
    nnode* left = m_node->left ; 
    nnode* right = m_node->right ; 

    if( type == CSG_DIFFERENCE )  // left - right 
    {
        a = left ; 
        b = right ; 
    }
    else if( type == CSG_INTERSECTION && !left->complement &&  right->complement)  // left * !right  ->   left - right
    { 
        a = left ; 
        b = right ; 
    }
    else if( type == CSG_INTERSECTION &&  left->complement && !right->complement)  // !left * right  ->  right - left 
    {
        a = right ; 
        b = left ; 
    }

    bool is_uncoincidable  =  a && b && a->type == CSG_BOX3 && b->type == CSG_BOX3 ;
    return is_uncoincidable ;
}

unsigned NNodeUncoincide::uncoincide_subtraction(nnode* a, nnode* b)
{
    float epsilon = 1e-5f ; 
    unsigned level = 1 ; 
    int margin = 1 ; 
    

    std::vector<nuv> coincident ;
    a->getCoincident( coincident, b, epsilon, level, margin, FRAME_LOCAL );

    unsigned ncoin = coincident.size() ;
    if(ncoin > 0)
    {
        LOG(info) << "NNodeUncoincide::uncoincide_subtraction   START " 
                  << " A " << a->tag()
                  << " B " << b->tag()
                  ;

        a->verbosity = 4 ; 
        b->verbosity = 4 ; 

        a->pdump("A");
        b->pdump("B");

        assert( ncoin == 1);
        nuv uv = coincident[0] ; 

        //float delta = 5*epsilon ; 
        float delta = 1 ;  // crazy large delta, so can see it  

        std::cout << "NNodeUncoincide::uncoincide_subtraction" 
                  << " ncoin " << ncoin 
                  << " uv " << uv.desc()
                  << " epsilon " << epsilon
                  << " delta " << delta
                  << std::endl 
                  ;

        b->nudge( uv.s(),  delta ); 

        b->pdump("B(nudged)");

        LOG(info) << "NNodeUncoincide::uncoincide   DONE " ; 
    }
    return ncoin  ;
}



/*

Hmm nothing fancy needed to see the coincidence,
equality of a.bbox.min.z and b.bbox.max.z or vv.
    
Shapes with separate +z and -z parameters are
easy to nudge in +z, -z direction.  

* CSG_CYLINDER
* CSG_CONE
* CSG_ZSPHERE

Shapes with symmetric parameters like box3 are a pain, as 
to grow in eg +z direction need to grow in both +z and -z
first and then transform to keep the other side at same place.

Hmm to avoid this perhaps make a CSG_ZBOX primitive ? 

*/


bool NNodeUncoincide::is_uncoincidable_union(nnode*& a, nnode*& b)
{
    OpticksCSG_t type = m_node->type ; 
    if( type != CSG_UNION ) return false ; 

    nnode* left = m_node->left ; 
    nnode* right = m_node->right ; 

    nbbox l = left->bbox();
    nbbox r = right->bbox();

    // order a,b primitives in increasing z
    //
    // The advantage of using bbox is that 
    // can check for bbox coincidence with all node shapes, 
    // not just the z-nudgeable which have z1() z2() methods.
    //

    // hmm these values sometimes have  transforms applied, 
    // so should use epsilon 

    float epsilon = 1e-5 ; 

    if( fabsf(l.max.z - r.min.z) < epsilon )  
    {
        LOG(info) << "   |----L----||--- R-------|  -> Z "    ;  
        a = left ; 
        b = right ;  
    }
    else if( fabsf(r.max.z - l.min.z )  < epsilon )    
    {
        LOG(info) << "   |----R----||---L-------|  -> Z "    ;  
        a = right ; 
        b = left ;  
    }

    bool is_uncoincidable = false ; 
    if( a && b )
    {
        bool can_fix = a->is_znudge_capable() && b->is_znudge_capable() ; 
        if(!can_fix) 
        {
            LOG(warning) << "bbox.z coincidence seen, but cannot fix as one/both primitives are not znudge-able " ;  
        } 
        else
        {
            LOG(warning) << "bbox.z coincidence seen, proceed to fix as both primitives are znudge-able " ;  
            is_uncoincidable = true ;  
        }
    }
    return is_uncoincidable ;
}


 
unsigned NNodeUncoincide::uncoincide_union(nnode* a, nnode* b)
{
    // opticks-tbool 143
    assert( a->is_znudge_capable() );
    assert( b->is_znudge_capable() );

    std::cout << std::endl << std::endl ; 
    LOG(info) << "NNodeUncoincide::uncoincide_union"
               << " A " << a->tag()
               << " B " << b->tag()
               ;

    a->dump("uncoincide_union A");
    b->dump("uncoincide_union B");

    /*

        +--------------+
        |              |
        |  A          ++-------------+
        |             ||             |
        |             ||          B  |
        |             ||             | 
        |             ||             |    
        |             ||             |
        |             ||             |
        |             ++-------------+
        |              |
        |              |
        +--------------+

       a.z1           a.z2
                      b.z1          b.z2      


       (schematic, actually there are usually transforms applied that
        prevent a.z2 == b.z1  ) 

        ------> Z

        Hmm in-principal the transforms could also change the radii 
        ordering but thats unlikely, as usually just translations.

    */


    float a_r2 = a->r2() ;
    float b_r1 = b->r1() ;

    float dz = 0.5 ;  // need some checks regarding size of the objs

    if( a_r2 > b_r1 )
    {
        b->decrease_z1( dz ); 
    }
    else
    {
        a->increase_z2( dz ); 
    }


    std::cout 
               << " A.z1 " << std::fixed << std::setw(10) << std::setprecision(4) << a->z1()
               << " A.z2 " << std::fixed << std::setw(10) << std::setprecision(4) << a->z2()
               << " B.z1 " << std::fixed << std::setw(10) << std::setprecision(4) << b->z1()
               << " B.z2 " << std::fixed << std::setw(10) << std::setprecision(4) << b->z2()
               << std::endl ; 

    std::cout 
               << " A.r1 " << std::fixed << std::setw(10) << std::setprecision(4) << a->r1()
               << " A.r2 " << std::fixed << std::setw(10) << std::setprecision(4) << a->r2()
               << " B.r1 " << std::fixed << std::setw(10) << std::setprecision(4) << b->r1()
               << " B.r2 " << std::fixed << std::setw(10) << std::setprecision(4) << b->r2()
               << std::endl ; 


    nbbox a_ = a->bbox();
    nbbox b_ = b->bbox();
    std::cout 
               << " a.bbox " << a_.desc() << std::endl 
               << " b.bbox " << b_.desc() << std::endl 
               ; 




    // to decide which one to nudge need to know the radius at the union interfaces
    // need to nudge the one with the smaller radius


    std::cout << std::endl << std::endl ; 
    //assert(0 && "hari-kari") ; 

    return 0 ; 
}








unsigned NNodeUncoincide::uncoincide_treewise()
{
    assert( m_node->is_root() );

    unsigned typmsk = m_node->get_type_mask();

    unsigned uncy     = CSGMASK_UNION | CSGMASK_CYLINDER ;
    unsigned uncyco   = CSGMASK_UNION | CSGMASK_CYLINDER | CSGMASK_CONE ;
    unsigned uncycodi = CSGMASK_UNION | CSGMASK_DIFFERENCE | CSGMASK_CYLINDER | CSGMASK_CONE ;

    bool root_difference = m_node->type == CSG_DIFFERENCE ; 
    bool root_uncy   = typmsk == uncy ;
    bool root_uncyco = typmsk == uncyco ;
    bool root_uncycodi = typmsk == uncycodi  ;

    if(root_uncy || root_uncyco)
    {
         uncoincide_uncyco(m_node);
    }
    else if( root_uncycodi )
    {
        nnode* left = m_node->left ; 
        unsigned left_typmsk = left->get_type_mask();

        //const nnode* right = m_node->right ; 
        //unsigned right_typmsk = right->get_type_mask();

        bool left_uncy   =  left_typmsk == uncy ;
        bool left_uncyco =  left_typmsk == uncyco ;

        if( root_difference  && ( left_uncy || left_uncyco ))
        {
            LOG(info) << "NNodeUncoincide::uncoincide_tree"
                      << " TRYING root.left UNCOINCIDE_UNCYCO " 
                      << " root " << m_node->get_type_mask_string()
                      << " left " << m_node->left->get_type_mask_string()
                      << " right " << m_node->right->get_type_mask_string()
                      ;

            uncoincide_uncyco( left );
        }
    }
    return 0 ; 
}






struct ZNudger 
{
    nnode* root ; 
    const float epsilon ; 
    const unsigned verbosity ; 
    unsigned znudge_count ; 

    std::vector<nnode*>       prim ; 
    std::vector<nbbox>        bb ; 
    std::vector<nbbox>        cc ; 
    std::vector<unsigned>     zorder ; 

    ZNudger(nnode* root, float epsilon, unsigned verbosity) 
         :
         root(root),
         epsilon(epsilon), 
         verbosity(verbosity),
         znudge_count(0)
    {
        init();
    }
   
    void init()
    {
         root->collect_prim_for_edit(prim);
         update_bb();
    }

    void update_bb()
    {
         zorder.clear();
         bb.clear(); 
         for(unsigned i=0 ; i < prim.size() ; i++)
         {
              const nnode* p = prim[i] ; 
              nbbox pbb = p->bbox(); 
              bb.push_back(pbb);
              zorder.push_back(i);
         }
         std::sort(zorder.begin(), zorder.end(), *this );
    } 

    bool operator()( int i, int j)  
    {
         return bb[i].min.z < bb[j].min.z ;    // ascending bb.min.z
    }  

    const char* UNCLASSIFIED_ = "UNCLASSIFIED" ; 
    const char* COINCIDENT_ = "COINCIDENT" ; 
    const char* OVERLAP_  = "OVERLAP" ; 
    const char* SPLIT_     = "SPLIT" ; 

    typedef enum {
        UNCLASSIFIED, 
        COINCIDENT, 
        OVERLAP, 
        SPLIT
    } Join_t ; 

    const char* join_type_string( Join_t join )
    {
        const char* s = NULL ; 
        switch(join)
        {
           case UNCLASSIFIED: s = UNCLASSIFIED_ ; break ; 
           case COINCIDENT: s = COINCIDENT_ ; break ; 
           case OVERLAP: s = OVERLAP_ ; break ; 
           case SPLIT: s = SPLIT_ ; break ; 
        }
        return s ; 
    } 

    Join_t join_type( float za, float zb )
    {
        float delta = zb - za   ; 
        Join_t join = UNCLASSIFIED ; 
 
        if( fabsf(delta) < epsilon )
        {
             join = COINCIDENT ; 
        } 
        else if( delta < 0.f )
        {
             join = OVERLAP ; 
        }
        else if( delta > 0.f )
        {
             join = SPLIT ; 
        }
        else
        {
             assert(0);
        } 
        return join ; 
    }



    void znudge()
    {
         int wid = 10 ;
         float dz = 1.0f ; // perhaps should depend on z-range of prims ?   
 
         if(verbosity > 0)
         LOG(info) << " znudge over prim pairs " 
                   << " dz " << dz
                    ; 

         for(unsigned i=1 ; i < prim.size() ; i++)
         {
              unsigned ja = zorder[i-1] ; 
              unsigned jb = zorder[i] ; 

              nnode* a = prim[ja] ;
              nnode* b = prim[jb] ;
 
              float za = bb[ja].max.z ; 
              float ra = a->r2() ; 

              float zb = bb[jb].min.z ; 
              float rb = b->r1() ; 
 
              Join_t join = join_type( za, zb );

              if(verbosity > 2)
              std::cout 
                     << " ja: " << std::setw(15) << prim[ja]->tag()
                     << " jb: " << std::setw(15) << prim[jb]->tag()
                     << " za: " << std::setw(wid) << std::fixed << std::setprecision(3) << za 
                     << " zb: " << std::setw(wid) << std::fixed << std::setprecision(3) << zb 
                     << " join " << std::setw(2*wid) << join_type_string(join)
                     << " ra: " << std::setw(wid) << std::fixed << std::setprecision(3) << ra 
                     << " rb: " << std::setw(wid) << std::fixed << std::setprecision(3) << rb 
                     << std::endl ; 

              if( join == COINCIDENT )
              {
                  // TODO: fix unjustified assumption that transforms dont swap the radii orderiing 
                  // expand side with smaller radii into the other to make the join OVERLAP
                  if( ra > rb )  
                  {
                      b->decrease_z1( dz );   
                  }
                  else
                  {
                      a->increase_z2( dz ); 
                  }
                  znudge_count++ ; 
              }  
         } 
         update_bb();
    }
    /*

        +--------------+ .
        |              |
        |           . ++-------------+
        |             ||             |
        |         rb  ||  ra         |
        |             ||             | 
        |           . || .           |    
        |             ||             |
        |             ||          b  |
        |           . ++-------------+
        |  a           |
        |              |
        +--------------+ .

                      za  
                      zb                      

        ------> Z

    */



    void dump(const char* msg="ZNudger::dump")
    {
          LOG(info) 
              << msg 
              << " treedir " << ( root->treedir ? root->treedir : "-" )
              << " typmsk " << root->get_type_mask_string() 
              << " nprim " << prim.size()
              << " znudge_count " << znudge_count
              << " verbosity " << verbosity
               ; 

          dump_qty('R');
          dump_qty('Z');
          dump_qty('B');
          dump_joins();
    }

    void dump_qty(char qty, int wid=10)
    {
         switch(qty)
         {
            case 'B': std::cout << "dump_qty : bbox (globally transformed) " << std::endl ; break ; 
            case 'Z': std::cout << "dump_qty : bbox.min/max.z (globally transformed) " << std::endl ; break ; 
            case 'R': std::cout << "dump_qty : model frame r1/r2 (local) " << std::endl ; break ; 
         }

         for(unsigned i=0 ; i < prim.size() ; i++)
         {
              unsigned j = zorder[i] ; 
              std::cout << std::setw(15) << prim[j]->tag() ;

              if(qty == 'Z' ) 
              {
                  for(unsigned indent=0 ; indent < i ; indent++ ) std::cout << std::setw(wid*2) << " " ;  
                  std::cout 
                        << std::setw(wid) << " bb.min.z " 
                        << std::setw(wid) << std::fixed << std::setprecision(3) << bb[j].min.z 
                        << std::setw(wid) << " bb.max.z " 
                        << std::setw(wid) << std::fixed << std::setprecision(3) << bb[j].max.z
                        << std::endl ; 
              } 
              else if( qty == 'R' )
              {
                  for(unsigned indent=0 ; indent < i ; indent++ ) std::cout << std::setw(wid*2) << " " ;  
                  std::cout 
                        << std::setw(wid) << " r1 " 
                        << std::setw(wid) << std::fixed << std::setprecision(3) << prim[j]->r1() 
                        << std::setw(wid) << " r2 " 
                        << std::setw(wid) << std::fixed << std::setprecision(3) << prim[j]->r2()
                        << std::endl ; 
              }
              else if( qty == 'B' )
              {
                   std::cout << bb[j].desc() << std::endl ; 
              }
         }
    }

    void dump_joins()
    {
         int wid = 10 ;
         std::cout << "dump_joins" << std::endl ; 

         for(unsigned i=1 ; i < prim.size() ; i++)
         {
              unsigned ja = zorder[i-1] ; 
              unsigned jb = zorder[i] ; 

              const nnode* a = prim[ja] ;
              const nnode* b = prim[jb] ;
 
              float za = bb[ja].max.z ; 
              float ra = a->r2() ; 

              float zb = bb[jb].min.z ; 
              float rb = b->r1() ; 
 
              Join_t join = join_type( za, zb );
              std::cout 
                     << " ja: " << std::setw(15) << prim[ja]->tag()
                     << " jb: " << std::setw(15) << prim[jb]->tag()
                     << " za: " << std::setw(wid) << std::fixed << std::setprecision(3) << za 
                     << " zb: " << std::setw(wid) << std::fixed << std::setprecision(3) << zb 
                     << " join " << std::setw(2*wid) << join_type_string(join)
                     << " ra: " << std::setw(wid) << std::fixed << std::setprecision(3) << ra 
                     << " rb: " << std::setw(wid) << std::fixed << std::setprecision(3) << rb 
                     << std::endl ; 
         }
     }
};

// end of ZNudger 


unsigned NNodeUncoincide::uncoincide_uncyco(nnode* node)
{
    float epsilon = 1e-5f ; 
    ZNudger zn(node, epsilon, m_verbosity) ; 

    if(m_verbosity > 2 )
    zn.dump("NNodeUncoincide::uncoincide_uncyco before znudge");

    zn.znudge();

    if(m_verbosity > 2 )
    zn.dump("NNodeUncoincide::uncoincide_uncyco after znudge");

    return 0 ; 
}


