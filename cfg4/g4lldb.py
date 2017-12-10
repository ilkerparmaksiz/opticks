#!/usr/bin/env python
"""
See
    (lldb) help br com add

    (lldb) script

    >>> help(lldb.frame)
    >>> help(lldb.SBValue)


Add the handler::

    (lldb) command script import opticks.cfg4.g4lldb
             ## put into ~/.lldbinit to avoid having to repeat this

    (lldb) b G4VDiscreteProcess::PostStepGetPhysicalInteractionLength 
             ## create pending breakpoint

    (lldb) br com  add 1 -F opticks.cfg4.g4lldb.brk    
             ## add command to pending breakpoint 


Run this script to update breakpoint setup commands, parsed 
from *lldb* lines in the comments of this script::

    ~/opticks/cfg4/g4lldb.py > /tmp/g4lldb.txt && cat /tmp/g4lldb.txt

    (lldb) command source /tmp/g4lldb.txt



"""

import os, sys, logging
from collections import defaultdict, OrderedDict

log = logging.getLogger(__name__)


FMT = "// %80s : %s " 
COUNT = defaultdict(lambda:0)

REGISTER = {}
 
class QDict(OrderedDict):

    @classmethod
    def Tag(cls, func):
        if func is None:return None
        pass
        name = "%s.%s" % ( __name__, func  )
        global COUNT 
        tag = "%s.[%d]" % ( name, COUNT[name] )
        COUNT[name] += 1 
        return tag

    def __init__(self,  this, pfx, func=None, label=""):
        OrderedDict.__init__(self)

        self.this = this
        self.pfx = pfx
        self.tag = self.Tag(func)
        self.label = label

        if self.MEMBERS is None:
            return
        pass
        for m in self.MEMBERS.split():
            mpfx = pfx+"->"+m
            raw = this.GetChildMemberWithName(m)
            kls = REGISTER.get(m, None) 
            self[m] = kls(raw, mpfx) if kls is not None else raw
        pass
    def __repr__(self):
        lines = []
        if self.tag is not None:
            lines.append("")
            lines.append(FMT % (self.tag, self.label))
        pass
        lines.append(FMT % (self.pfx, self.__class__.__name__))
        for k,v in self.items():
            brief = getattr(v, "BRIEF", False)
            composite = v.__class__ in REGISTER.values()
            if brief:
                lines.append(FMT % (k,v))
            elif composite:
                lines.append(FMT % (k, ""))
                lines.append("%s" % v)
            else:
                lines.append(FMT % (k,v))
            pass
        pass
        return "\n".join(lines)


class V(object):
    def __init__(self, v):
        self.v = v
    def __repr__(self):
        return self.v.GetValue()


class G4ThreeVector(QDict):
    BRIEF = True
    MEMBERS = "dx dy dz" 
    def __repr__(self):
        return  " (%8.3f %8.3f %8.3f) " % ( float(self["dx"].GetValue()), float(self["dy"].GetValue()), float(self["dz"].GetValue()) )
        
class G4double(QDict):
    BRIEF = True
    MEMBERS = None
    def __repr__(self):
        return " %8.3f " % float(self.this.GetValue())
    
      
class Enum(QDict):
    BRIEF = True
    MEMBERS = None
    def __repr__(self):
        return " %s " % self.this.GetValue()


class G4StepPoint(QDict):
    MEMBERS = "fPosition fGlobalTime fMomentumDirection fPolarization fVelocity"

class G4Step(QDict):
    MEMBERS = "fpPreStepPoint fpPostStepPoint"

class G4SteppingManager(QDict):
    MEMBERS = "fStep fStepStatus PhysicalStep fCurrentProcess"

class G4TrackingManager(QDict):
    MEMBERS = "fpSteppingManager"

class G4VProcess(QDict):
    MEMBERS = "theProcessName" 

class CRandomEngine(QDict):
    MEMBERS = "m_flat m_current_record_flat_count" 

    def __repr__(self):
        brief = "%15s %s " % ( self["m_flat"].GetValue(), self["m_current_record_flat_count"].GetValue())
        return FMT % (self.tag, brief)


REGISTER["fpPreStepPoint"] = G4StepPoint 
REGISTER["fpPostStepPoint"] = G4StepPoint 
REGISTER["fStep"] = G4Step 
REGISTER["fpSteppingManager"] = G4SteppingManager 
REGISTER["fCurrentProcess"] = G4VProcess 
REGISTER["fPosition"] = G4ThreeVector
REGISTER["fMomentumDirection"] = G4ThreeVector
REGISTER["fPolarization"] = G4ThreeVector
REGISTER["fGlobalTime"] = G4double
REGISTER["fVelocity"] = G4double
REGISTER["PhysicalStep"] = G4double
REGISTER["fStepStatus"] = Enum




def CRandomEngine_cc_210(frame, bp_loc, sess):
    """   
    (*lldb*) br set -f CRandomEngine.cc -l 210
    (*lldb*) br com add 1 -F opticks.cfg4.g4lldb.CRandomEngine_cc_210
    """
    engine = CRandomEngine(frame.FindVariable("this") , "this", func=sys._getframe().f_code.co_name, label="-") 
    print engine
    return False



def G4TrackingManager_cc_131(frame, bp_loc, sess):
    """
    (*lldb*) br set -f G4TrackingManager.cc -l 131
    (*lldb*) br com add 1 -F opticks.cfg4.g4lldb.G4TrackingManager_cc_131

    ::

        110   // Give SteppingManger the maxmimum number of processes 
        111   fpSteppingManager->GetProcessNumber();
        112 
        113   // Give track the pointer to the Step
        114   fpTrack->SetStep(fpSteppingManager->GetStep());
        115 
        116   // Inform beginning of tracking to physics processes 
        117   fpTrack->GetDefinition()->GetProcessManager()->StartTracking(fpTrack);
        118 
        119   // Track the particle Step-by-Step while it is alive
        120   //  G4StepStatus stepStatus;
        121 
        122   while( (fpTrack->GetTrackStatus() == fAlive) ||
        123          (fpTrack->GetTrackStatus() == fStopButAlive) ){
        124 
        125     fpTrack->IncrementCurrentStepNumber();
        126     fpSteppingManager->Stepping();
        127 #ifdef G4_STORE_TRAJECTORY
        128     if(StoreTrajectory) fpTrajectory->
        129                         AppendStep(fpSteppingManager->GetStep());
        130 #endif
        131     if(EventIsAborted) {
        132       fpTrack->SetTrackStatus( fKillTrackAndSecondaries );
        133     }
        134   }
        135   // Inform end of tracking to physics processes 
        136   fpTrack->GetDefinition()->GetProcessManager()->EndTracking();

    """
    trackMgr = G4TrackingManager(frame.FindVariable("this") , "this", func=sys._getframe().f_code.co_name, label="after-Stepping") 
    print trackMgr
    return False

def G4SteppingManager_cc_191(frame, bp_loc, sess):
    """
    (lldb) b -f G4SteppingManager.cc -l 191
    (lldb) br com add 1 -F opticks.cfg4.g4lldb.G4SteppingManager_cc_191

    g4-;g4-cls G4SteppingManager 
    """
    stepMgr = G4SteppingManager(frame.FindVariable("this") , "this", func=sys._getframe().f_code.co_name, label="end-step") 
    print stepMgr
    return True


def G4SteppingManager2_cc_181_DefinePhysicalStepLength(frame, bp_loc, sess):
    """

    See:  notes/issues/stepping_process_review.rst 

    ::

        g4-;g4-cls G4SteppingManager 
        g4-;g4-cls G4SteppingManager2
 
        tboolean-;tboolean-box --okg4 --align -D


        (lldb) b -f G4SteppingManager2.cc -l 181

            ## inside process loop after PostStepGPIL call giving physIntLength and fCondition

        (lldb) br com  add 1 -F opticks.cfg4.g4lldb.G4SteppingManager2_cc_181_DefinePhysicalStepLength


    Seems can access member vars, but so far not general stack items, other than "this" ?
    """
    name = "%s.%s " % ( __name__, sys._getframe().f_code.co_name  )
    global COUNT 

    kvar = "physIntLength fCondition PhysicalStep fStepStatus fPostStepDoItProcTriggered"

    this = frame.FindVariable("this")
    proc = this.GetChildMemberWithName("fCurrentProcess")
    procName = proc.GetChildMemberWithName("theProcessName")

    print "//" 
    print FMT % ( name, COUNT[name] )
    print FMT % ( "procName", procName ) 
    COUNT[name] += 1 

    for k in kvar.split():
        #v = frame.FindVariable(k)    gives no-value
        v = this.GetChildMemberWithName(k)
        print FMT % ( k, v )
    pass
    return False




def py_G4VDiscreteProcess_PostStepGetPhysicalInteractionLength(frame, bp_loc, sess):
    """
    ::

        b G4VDiscreteProcess::PostStepGetPhysicalInteractionLength
        br com  add 1 -F opticks.cfg4.g4lldb.py_G4VDiscreteProcess_PostStepGetPhysicalInteractionLength

    """
    name = "py_G4VDiscreteProcess_PostStepGetPhysicalInteractionLength"
    proc = frame.FindVariable("this")
    procName = proc.GetChildMemberWithName("theProcessName")
    left = proc.GetChildMemberWithName("theNumberOfInteractionLengthLeft")
    print "%100s %s %s  " % ( name, procName, left )
    return False


def py_G4VProcess_ResetNumberOfInteractionLengthLeft(frame, bp_loc, sess):
    """

        b G4VProcess::ResetNumberOfInteractionLengthLeft
        br com  add 1 -F opticks.cfg4.g4lldb.py_G4VProcess_ResetNumberOfInteractionLengthLeft    
 
    """
    name = "py_G4VProcess_ResetNumberOfInteractionLengthLeft"
    this = frame.FindVariable("this")
    procName = this.GetChildMemberWithName("theProcessName")
    print "%100s %s " % ( name, procName )
    return False


class Mock(object):
    def __init__(self):
        pass
    def GetChildMemberWithName(self, name):
        return name


def test_G4StepPoint():
    dummy = Mock() 
    sp = G4StepPoint(dummy,"dummy")
    print sp


class Parse(dict):
    PREFIX = "    (*lldb*)"
    BR_SET = "br set"
    BR_COM_ADD_1_ = "br com add 1 "
    def __init__(self):
        dict.__init__(self)
        self["N"] = 0 
        lines = filter(lambda l:l.startswith(self.PREFIX), file(__file__).readlines())
        lines = map(lambda l:l[len(self.PREFIX):].rstrip().lstrip(), lines)
        self.lines = []
        for line in lines:
            is_br_set = line.startswith(self.BR_SET)
            is_br_com_add = line.startswith(self.BR_COM_ADD_1_)
            #print "# is_br_set:%d is_br_com_add:%d  : %s " % ( is_br_set, is_br_com_add, line )
            if is_br_set:self["N"] += 1 
            if is_br_com_add:line = line.replace(" 1 ", " %(N)s ")
            pass
            self.lines.append( line % self )
        pass
    def __repr__(self):
        hdr1 = "# generated from-and-by %s " % ( os.path.abspath(__file__) )  
        hdr2 = "# command source /tmp/g4lldb.txt "
        return "\n".join([hdr1,hdr2]+self.lines)
       

 
if __name__ == '__main__':
    print Parse()

