/**
sreport.cc : Summarize + Present SEvt/NPFold metadata time stamps 
=============================================================================

::
 
    ~/opticks/sysrap/tests/sreport.sh 
    ~/opticks/sysrap/tests/sreport.sh grab
    ~/opticks/sysrap/tests/sreport.sh ana 


Summarizes SEvt/NPFold metadata time stamps into substamp arrays 
grouped by NPFold path prefix. The summary NPFold is presented textually 
and saved to allow plotting from python. 

+-----+---------------------------------+-------------------------+
| key | SEvt/NPFold path prefix         |  SEvt type              |
+=====+=================================+=========================+
| a   | "//p" eg: //p001 //p002         | Opticks/QSim SEvt       |
+-----+---------------------------------+-------------------------+
| b   | "//n" eg: //n001 //n002         | Geant4/U4Recorder SEvt  |
+-----+---------------------------------+-------------------------+

The tables are presented with row and column labels and the 
summary NPFold is saved to DIR_sreport sibling to invoking DIR 
which needs to contain SEvt/NPFold folders corresponding to the path prefix.  
The use of NPFold::LoadNoData means that only SEvt NPFold/NP 
metadata is loaded. Excluding the array data makes the load 
very fast and able to handle large numbers of persisted SEvt NPFold.

Usage from source "run" directory creates the report saving into eg ../ALL3_sreport::

    epsilon:~ blyth$ cd /data/blyth/opticks/GEOM/J23_1_0_rc3_ok0/G4CXTest/ALL3
    epsilon:ALL3 blyth$ sreport 
    epsilon:ALL3 blyth$ ls -alst ../ALL3_sreport 

Usage from report directory loads and presents the report::

    epsilon:ALL3 blyth$ cd ../ALL3_sreport/
    epsilon:ALL3_sreport blyth$ sreport        

Note that this means that can only rsync the small report directory 
and still be able to present the report on laptop concerening run folders
with many large arrays left on the server. 

**/

#include "NPFold.h"


struct sreport
{
    static constexpr const char* JUNCTURE = "SEvt__Init_RUN_META,SEvt__BeginOfRun,SEvt__EndOfRun,SEvt__Init_RUN_META" ; 
    static constexpr const char* RANGES = R"(
        SEvt__Init_RUN_META:CSGFoundry__Load_HEAD       
        CSGFoundry__Load_HEAD:CSGFoundry__Load_TAIL     ## load geom 
        CSGOptiX__Create_HEAD:CSGOptiX__Create_TAIL     ## upload geom
        QSim__simulate_HEAD:QSim__simulate_PREL         ## upload genstep
        QSim__simulate_PREL:QSim__simulate_POST         ## simulate kernel
        QSim__simulate_POST:QSim__simulate_TAIL         ## download 
        QSim__simulate_TAIL:CSGOptiX__SimulateMain_TAIL
       )" ; 

    bool    VERBOSE ;

    NP*       run ; 
    NP*       runprof ;
    NPFold*   substamp ;   
    NPFold*   subprofile ;   

    sreport();  

    NPFold* serialize() const ; 
    void    import( const NPFold* fold ); 
    void save(const char* dir) const ; 
    static sreport* Load(const char* dir) ; 

    std::string desc() const ;
    std::string desc_run() const ;
    std::string desc_runprof() const ;
    std::string desc_substamp() const ;
    std::string desc_subprofile() const ;
};

inline sreport::sreport()
    :
    VERBOSE(getenv("sreport__VERBOSE") != nullptr),
    run( nullptr ),
    runprof( nullptr ),
    substamp( nullptr),
    subprofile( nullptr )
{
}
inline NPFold* sreport::serialize() const 
{
    NPFold* smry = new NPFold ;  
    smry->add("run", run ) ; 
    smry->add("runprof", runprof ) ; 
    smry->add_subfold("substamp", substamp ) ; 
    smry->add_subfold("subprofile", subprofile ) ; 
    return smry ; 
}
inline void sreport::import(const NPFold* smry) 
{
    run = smry->get("run")->copy() ; 
    runprof = smry->get("runprof")->copy() ; 
    substamp = smry->get_subfold("substamp"); 
    subprofile = smry->get_subfold("subprofile"); 
}
inline void sreport::save(const char* dir) const 
{
    NPFold* smry = serialize(); 
    smry->save_verbose(dir); 
}
inline sreport* sreport::Load(const char* dir) // static
{
    NPFold* smry = NPFold::Load(dir) ; 
    sreport* report = new sreport ; 
    report->import(smry) ; 
    return report ; 
}

inline std::string sreport::desc() const
{
    std::stringstream ss ; 
    ss << "[sreport.desc" << std::endl 
       << desc_run() 
       << desc_runprof() 
       << desc_substamp()
       << "]sreport.desc" << std::endl 
       ; 
    std::string str = ss.str() ;
    return str ;  
}

inline std::string sreport::desc_run() const
{
    std::stringstream ss ; 
    ss << "[sreport.desc_run" << std::endl 
       << ( run ? run->sstr() : "-" ) << std::endl 
       << ".sreport.desc_run.descMetaKVS " << std::endl 
       << ( run ? run->descMetaKVS(JUNCTURE, RANGES) : "-" ) << std::endl
       << "]sreport.desc_run" << std::endl 
       ; 
    std::string str = ss.str() ;
    return str ;  
}


inline std::string sreport::desc_runprof() const
{
    std::stringstream ss ; 
    ss << "[sreport.desc_runprof" << std::endl 
       << ( runprof ? runprof->sstr() : "-" ) << std::endl 
       << ".sreport.desc_runprof.descTable " << std::endl 
       << ( runprof ? runprof->descTable<int64_t>(17) : "-" ) << std::endl
       << "]sreport.desc_runprof" << std::endl 
       ; 
    std::string str = ss.str() ;
    return str ;  
}

inline std::string sreport::desc_substamp() const
{
    std::stringstream ss ; 
    ss << "[sreport.desc_substamp" << std::endl 
       ;
    if(VERBOSE) ss
       << "[sreport.desc_substamp.VERBOSE" << std::endl 
       << ( substamp ? substamp->desc() : "-" )
       << "]sreport.desc_substamp.VERBOSE" << std::endl 
       ;

    ss << "[sreport.desc_substamp.compare_subarrays_report" << std::endl 
       <<  ( substamp ? substamp->compare_subarrays_report<double, int64_t>( "delta_substamp", "a", "b" ) : "-" )
       << "]sreport.desc_substamp.compare_subarrays_report" << std::endl 
       << "]sreport.desc_substamp" << std::endl 
       ; 
    std::string str = ss.str() ;
    return str ;  
}


inline std::string sreport::desc_subprofile() const
{
    std::stringstream ss ; 
    ss << "[sreport.desc_subprofile" << std::endl 
       ;
    if(VERBOSE) ss
       << "[sreport.desc_subprofile.VERBOSE" << std::endl 
       << ( subprofile ? subprofile->desc() : "-" )
       << "]sreport.desc_subprofile.VERBOSE" << std::endl 
       ;

    /*
    ss << "[sreport.desc_subprofile.compare_subarrays_report" << std::endl 
       <<  ( subprofile ? subprofile->compare_subarrays_report<double, int64_t>( "delta_subprofile", "a", "b" ) : "-" )
       << "]sreport.desc_subprofile.compare_subarrays_report" << std::endl 
    */

    ss
       << "]sreport.desc_subprofile" << std::endl 
       ; 
    std::string str = ss.str() ;
    return str ;  
}



struct sreport_Creator
{ 
    static constexpr const char* ASEL = "a://A" ; 
    static constexpr const char* BSEL = "b://B" ; 

    bool VERBOSE ;
    const char* dirp ; 
    NPFold*    fold ; 
    bool fold_valid ; 
    const NP*  run ; 
    sreport*   report ; 

    sreport_Creator(  const char* dirp_ ); 
    void init(); 

    std::string desc() const ;
    std::string desc_fold() const ;
    std::string desc_fold_detail() const ;
    std::string desc_run() const ;
}; 

inline sreport_Creator::sreport_Creator( const char* dirp_ )
    :
    VERBOSE(getenv("sreport_Creator__VERBOSE") != nullptr),
    dirp(dirp_ ? strdup(dirp_) : nullptr),
    fold(NPFold::LoadNoData(dirp)),
    fold_valid(NPFold::IsValid(fold)),
    run(fold_valid ? fold->get("run") : nullptr),
    report(new sreport)
{
    init(); 
}

inline void sreport_Creator::init() 
{
    report->run     = run ? run->copy() : nullptr ; 
    report->runprof = run ? run->makeMetaKVProfileArray("Index") : nullptr ; 
    if(run) NP::CopyMeta( report->runprof, run ) ;   
    report->substamp   = fold_valid ? fold->subfold_summary("substamp",   ASEL, BSEL) : nullptr ; 
    report->subprofile = fold_valid ? fold->subfold_summary("subprofile", ASEL, BSEL) : nullptr ; 
}

inline std::string sreport_Creator::desc() const
{
    std::stringstream ss ; 
    ss << "[sreport_Creator.desc" << std::endl 
       << desc_fold() 
       << ( VERBOSE ? desc_fold_detail() : "" ) 
       << ( VERBOSE ? desc_run() : "" )
       << "]sreport_Creator.desc" << std::endl 
       ; 
    std::string str = ss.str() ;
    return str ;  
}

inline std::string sreport_Creator::desc_fold() const
{
    std::stringstream ss ; 
    ss << "[sreport_Creator.desc_fold" << std::endl 
       << "fold = NPFold::LoadNoData(\"" << dirp << "\")" << std::endl
       << "fold " << ( fold ? "YES" : "NO " )  << std::endl
       << "fold_valid " << ( fold_valid ? "YES" : "NO " ) << std::endl
       << "]sreport_Creator.desc_fold" << std::endl 
       ; 
    std::string str = ss.str() ;
    return str ;  
}

inline std::string sreport_Creator::desc_fold_detail() const
{
    std::stringstream ss ; 
    ss
       << "[sreport_Creator.desc_fold_detail " << std::endl
       << ( fold ? fold->desc() : "-" ) << std::endl
       << "]sreport_Creator.desc_fold_detail " << std::endl
       ; 
    std::string str = ss.str() ;
    return str ;  
}

inline std::string sreport_Creator::desc_run() const
{
    std::stringstream ss ; 
    ss << "[sreport_Creator.desc_run" << std::endl 
       << ( run ? run->sstr() : "-" ) << std::endl 
       << ".sreport_Creator.desc_run.descMetaKVS " << std::endl 
       << ( run ? run->descMetaKVS() : "-" ) << std::endl
       << "]sreport_Creator.desc_run" << std::endl 
       ; 
    std::string str = ss.str() ;
    return str ;  
}




int main(int argc, char** argv)
{
    char* argv0 = argv[0] ; 
    const char* dirp = argc > 1 ? argv[1] : U::PWD() ;   
    if(dirp == nullptr) return 0 ; 
    bool is_executable_sibling_path = U::IsExecutableSiblingPath( argv0 , dirp ) ; 

    std::cout 
       << "[sreport.main"
       << "  argv0 " << ( argv0 ? argv0 : "-" )
       << " dirp " << ( dirp ? dirp : "-" ) 
       << " is_executable_sibling_path " << ( is_executable_sibling_path ? "YES" : "NO " ) 
       << std::endl 
       ; 

    if( is_executable_sibling_path == false )  // not in eg ALL3_sreport directory 
    {
        U::SetEnvDefaultExecutableSiblingPath("FOLD", argv0, dirp );
        std::cout << "[sreport.main : CREATING REPORT " << std::endl ; 
        sreport_Creator creator(dirp); 
        std::cout << creator.desc() ; 
        if(!creator.fold_valid) return 1 ; 

        sreport* report = creator.report ; 
        std::cout << report->desc() ; 
        report->save("$FOLD"); 
        std::cout << "]sreport.main : CREATED REPORT " << std::endl ; 

        if(getenv("CHECK") != nullptr )
        {        
            std::cout << "[sreport.main : CHECK LOADED REPORT " << std::endl ; 
            sreport* report2 = sreport::Load("$FOLD") ;  
            std::cout << report2->desc() ; 
            std::cout << "]sreport.main : CHECK LOADED REPORT " << std::endl ; 
        }

    }
    else
    {
        std::cout << "[sreport.main : LOADING REPORT " << std::endl ; 
        sreport* report = sreport::Load(dirp) ;  
        std::cout << report->desc() ; 
        std::cout << "]sreport.main : LOADED REPORT " << std::endl ; 
    }

    std::cout 
       << "]sreport.main"
       << std::endl
       ;

    return 0 ; 
}

