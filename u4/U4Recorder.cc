#include "PLOG.hh"
#include "U4Recorder.hh"
#include "U4Track.h"


const plog::Severity U4Recorder::LEVEL = PLOG::EnvLevel("U4Recorder", "DEBUG"); 

U4Recorder* U4Recorder::INSTANCE = nullptr ; 
U4Recorder* U4Recorder::Get(){ return INSTANCE ; }



U4Recorder::U4Recorder()
{
    INSTANCE = this ; 
}

void U4Recorder::BeginOfRunAction(const G4Run*)
{
    LOG(info); 
}
void U4Recorder::EndOfRunAction(const G4Run*)
{
    LOG(info); 
}
void U4Recorder::BeginOfEventAction(const G4Event*)
{
    LOG(info); 
}
void U4Recorder::EndOfEventAction(const G4Event*)
{
    LOG(info); 
}
void U4Recorder::PreUserTrackingAction(const G4Track* track)
{
    LOG(info) << U4Track::Desc(track) ; 

    spho sp = U4Track::Label(track); 
    if(sp.isDefined()) labels.push_back(sp); 
}

void U4Recorder::PostUserTrackingAction(const G4Track*)
{
    //LOG(info); 
}
void U4Recorder::UserSteppingAction(const G4Step*)
{
    //LOG(info); 
}

