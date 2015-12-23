#include "OpIndexer.hh"

#include <algorithm>
#include <cassert>

// npy-
#include "NLog.hpp"
#include "Timer.hpp"
#include "NumpyEvt.hpp"  
#include "NPY.hpp"  

// cudawrap-
#include "CResource.hh"
#include "CBufSpec.hh"

// thrustrap-
#include "TBuf.hh"
#include "TSparse.hh"

// optixrap-
#include "OBuf.hh"


void OpIndexer::init()
{
    LOG(info) << "OpIndexer::init" ;
    m_timer      = new Timer("OpIndexer::");
    m_timer->setVerbose(true);
}

void OpIndexer::setEvt(NumpyEvt* evt)
{
    m_evt = evt ; 
}

void OpIndexer::updateEvt()
{
    m_phosel = m_evt->getPhoselData(); 
    m_recsel = m_evt->getRecselData();
    m_maxrec = m_evt->getMaxRec(); 
    m_sequence = m_evt->getSequenceData();
}

void OpIndexer::indexSequence()
{
    if(!m_evt) return ; 

    updateEvt();
    if(m_seq)
    {
        indexSequenceOptiXThrust();
    }
    else
    {
        indexSequenceGLThrust();
    }
}

void OpIndexer::indexSequenceOptiXThrust()
{
    LOG(info) << "OpIndexer::indexSequenceOptiXThrust" ; 
    CBufSlice seqh = m_seq->slice(2,0) ;  // stride, begin
    CBufSlice seqm = m_seq->slice(2,1) ;
    indexSequence(seqh, seqm, true);
}

void OpIndexer::indexSequenceGLThrust()
{
    // loaded does not involve OptiX, so there is no OBuf 
    // instead grab the sequence 
    LOG(info) << "OpIndexer::indexSequenceGLThrust" ; 

    CResource rsequence( m_sequence->getBufferId(), CResource::W );
    {
        TBuf tsequence("tsequence", rsequence.mapGLToCUDA<unsigned long long>() );
        CBufSlice seqh = tsequence.slice(2,0) ; // stride, begin  
        CBufSlice seqm = tsequence.slice(2,1) ;
        indexSequence(seqh, seqm, true);
    }
    rsequence.unmapGLToCUDA(); 
}

void OpIndexer::indexSequence(const CBufSlice& seqh, const CBufSlice& seqm, bool verbose )
{
    m_timer->start();
    TSparse<unsigned long long> seqhis("History_Sequence", seqh );
    TSparse<unsigned long long> seqmat("Material_Sequence", seqm ); 
    m_evt->setHistorySeq(seqhis.getIndex());
    m_evt->setMaterialSeq(seqmat.getIndex());  // the indices are populated the the make_lookup below

    CResource rphosel( m_phosel->getBufferId(), CResource::W );
    CResource rrecsel( m_recsel->getBufferId(), CResource::W );
    {
        TBuf tphosel("tphosel", rphosel.mapGLToCUDA<unsigned char>() );
        tphosel.zero();

        TBuf trecsel("trecsel", rrecsel.mapGLToCUDA<unsigned char>() );
        if(verbose) dump(tphosel, trecsel);

        seqhis.make_lookup(); 

        // phosel buffer is shaped (num_photons, 1, 4)
        CBufSlice tp_his = tphosel.slice(4,0) ; // stride, begin  
        CBufSlice tp_mat = tphosel.slice(4,1) ; 
      
        seqhis.apply_lookup<unsigned char>(tp_his); 
        if(verbose) dumpHis(tphosel, seqhis) ;

        seqmat.make_lookup();
        seqmat.apply_lookup<unsigned char>(tp_mat);
        if(verbose) dumpMat(tphosel, seqmat) ;

        tphosel.repeat_to<unsigned char>( &trecsel, 4, 0, tphosel.getSize(), m_maxrec );  // other, stride, begin, end, repeats

        tphosel.download<unsigned char>( m_phosel );  // cudaMemcpyDeviceToHost
        trecsel.download<unsigned char>( m_recsel );
    }
    rphosel.unmapGLToCUDA(); 
    rrecsel.unmapGLToCUDA(); 

    m_timer->stop();

    (*m_timer)("indexSequence"); 
}


void OpIndexer::dumpHis(const TBuf& tphosel, const TSparse<unsigned long long>& seqhis)
{
    OBuf* seq = m_seq ; 
    if(seq)
    {
        unsigned int nsqa = seq->getNumAtoms(); 
        unsigned int nsqd = std::min(nsqa,100u); 
        seq->dump<unsigned long long>("OpIndexer::dumpHis seq(2,0)", 2, 0, nsqd);
    }

    unsigned int nphosel = tphosel.getSize() ; 
    unsigned int npsd = std::min(nphosel,100u) ;
    tphosel.dumpint<unsigned char>("tphosel.dumpint<unsigned char>(4,0)", 4,0, npsd) ;
    LOG(info) << seqhis.dump_("OpIndexer::dumpHis seqhis");
}

void OpIndexer::dumpMat(const TBuf& tphosel, const TSparse<unsigned long long>& seqmat)
{
    OBuf* seq = m_seq ; 
    if(seq) 
    {
        unsigned int nsqa = seq->getNumAtoms(); 
        unsigned int nsqd = std::min(nsqa,100u); 
        seq->dump<unsigned long long>("OpIndexer::dumpMat OBuf seq(2,1)", 2, 1, nsqd);
    }

    unsigned int nphosel = tphosel.getSize() ; 
    unsigned int npsd = std::min(nphosel,100u) ;
    tphosel.dumpint<unsigned char>("tphosel.dumpint<unsigned char>(4,1)", 4,1, npsd) ;
    LOG(info) << seqmat.dump_("OpIndexer::dumpMat seqmat");
}

void OpIndexer::dump(const TBuf& tphosel, const TBuf& trecsel)
{
    OBuf* seq = m_seq ; 

    unsigned int nphosel = tphosel.getSize() ; 
    unsigned int nrecsel = trecsel.getSize() ; 

    LOG(info) << "OpIndexer::dump"
              << " nphosel " << nphosel
              << " nrecsel " << nrecsel
              ; 

    if(seq)
    {
        unsigned int nsqa = seq->getNumAtoms(); 
        assert(nphosel == 2*nsqa);
        assert(nrecsel == m_maxrec*2*nsqa);
    } 
}



void OpIndexer::saveSel()
{
    m_phosel->save("/tmp/phosel.npy");  
    m_recsel->save("/tmp/recsel.npy");  
}

