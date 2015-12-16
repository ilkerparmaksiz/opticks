#include "Opticks.hh"
#include "OpticksCfg.hh"

#include "NLog.hpp"

int main(int argc, char** argv)
{
    Opticks* opticks = new Opticks();

    Cfg* cfg  = new Cfg("umbrella", false) ;

    Cfg* ocfg = new OpticksCfg<Opticks>("opticks", opticks,false);

    cfg->add(ocfg);

    cfg->commandline(argc, argv);

    LOG(info) << cfg->getDesc() ;



    return 0 ; 
}
