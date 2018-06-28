/*

  op --dsst --gmeshlib --dbgmesh near_top_cover_box0xc23f970  
  op --dsst --gmeshlib --dbgmesh near_top_cover_box0x


  dsst
      sets geometry selection envvars, defining the path to the geocache
  gmeshlib
      used by op script to pick this executable GMeshLibTest 
  dbgmesh
      name of mesh to dump 



*/

#include "Opticks.hh"
#include "GMeshLib.hh"
#include "GMesh.hh"

#include "OPTICKS_LOG.hh"

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    Opticks ok(argc, argv);
    ok.configure();
    //ok.dump("after configure");

    bool analytic = false ; 
    GMeshLib* ml = GMeshLib::Load(&ok, analytic);

    const char* dbgmesh = ok.getDbgMesh();

    if(dbgmesh)
    {
        bool startswith = true ; 
        const GMesh* mesh = ml->getMesh(dbgmesh, startswith);
        mesh->dump("GMesh::dump", 50);
    }
    else
    {
        LOG(info) << "no dbgmesh" ; 
        ml->dump();
    }


    return 0 ; 
}

