#include "TDbiDatabaseManager.hxx"
#include "TDbiCascader.hxx"
#include "TSeqNoAllocator.hxx"

// Issue local and global SEQNOs.

Int_t CP::TSeqNoAllocator::GetSeqNo(const std::string& tableName,
                                    Int_t requireGlobal /* = 0 */,
                                    Int_t dbNo          /* = 0 */) const
//   tableName       in    The table for which the SEQNO is required.
//   requireGlobal   in    The type of SEQNO required:-
//                           > 0  Must be global
//                           = 0  Must be global if supplied dbNo is authorising
//                                and table isn't temporary otherwise local
//                           < 0  Must be local
//   dbNo            in     The entry in the cascade for which the SEQNO is required

{
   return CP::TDbiDatabaseManager::Instance().GetCascader().AllocateSeqNo(tableName,requireGlobal,dbNo);

}


