#include "TDbiDatabaseManager.hxx"
#include "TDbiCascader.hxx"
#include "TSeqNoAllocator.hxx"

/// Provide a global sequence number for a table name.  The requireGlobal
/// parameter is interpreted as the type of SEQNO to return: > 0 Must be
/// global, = 0 Must be global if supplied dbNo is authorising and table isn't
/// temporary otherwise local, < 0 Must be local.  The dbNo is the entry in
/// the cascade for which the SEQNO is required
Int_t CP::TSeqNoAllocator::GetSeqNo(const std::string& tableName,
                                    Int_t requireGlobal /* = 0 */,
                                    Int_t dbNo          /* = 0 */) const
{
    return CP::TDbiDatabaseManager::Instance()
        .GetCascader()
        .AllocateSeqNo(tableName,requireGlobal,dbNo);

}


