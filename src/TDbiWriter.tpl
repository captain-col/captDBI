///////////////////////////////////////////////////////////////////////
// $id: TDbiWriter.tpl,v 1.11 2005/11/11 09:27:26 west Exp $
//
// ND::TDbiWriter
//
// Package: ND::TDbi (Database Interface).

#include <cassert>
#include <fstream>

#include "TDbiCascader.hxx"
#include "TDbiOutRowStream.hxx"
#include "TDbiResultSetHandle.hxx"
#include "TDbiSqlValPacket.hxx"
#include "TDbiTableProxy.hxx"
#include "TDbiWriter.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;


ClassImpT(ND::TDbiWriter,T)

//   Definition of static data members
//   *********************************

//CVSID("$Id: TDbiWriter.tpl,v 1.1 2011/01/18 05:49:20 finch Exp $");

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

template<class T>
ND::TDbiWriter<T>::TDbiWriter() :
fAggregateNo(-2),
fDbNo(0),
fPacket(new ND::TDbiSqlValPacket),
fRequireGlobalSeqno(0),
fTableProxy(&ND::TDbiWriter<T>::GetTableProxy()),
fTableName(fTableProxy->GetTableName()),
fUseOverlayCreationDate(kFALSE),
fValidRec(0),
fLogEntry(fTableName)
{
//
//  Purpose:  Default Constructor
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create Writer.


//  Program Notes:-
//  =============

//  Creates an unusable ND::TDbiWriter - only present to keep rootcint happy.


  DbiTrace(  "Creating default ND::TDbiWriter for " << fTableName << "  ");

}
//.....................................................................

template<class T>
ND::TDbiWriter<T>::TDbiWriter(const ND::TVldRange& vr,
                        Int_t aggNo,
                        TDbi::Task task,
                        ND::TVldTimeStamp creationDate,
                        UInt_t dbNo,
                        const std::string& logComment,
                        const std::string& tableName) :
fAggregateNo(aggNo),
fDbNo(dbNo),
fPacket(new ND::TDbiSqlValPacket),
fRequireGlobalSeqno(0),
fTableProxy(&ND::TDbiWriter<T>::GetTableProxy(tableName)),
fTableName(fTableProxy->GetTableName()),
fUseOverlayCreationDate(creationDate == ND::TVldTimeStamp(0,0)),
fValidRec(0),
fLogEntry(fTableName,logComment,vr.GetDetectorMask(),vr.GetSimMask(),task)
{
//
//  Purpose:  Constructor
//
//  Arguments:
//    vr            in       Validity range of validity set.
//    aggNo         in       Aggregate number of validity set.
//    task          in       Task of validity set.
//    creationDate  in       Creation date of validity set.
//    dbNo          in       Database number in cascade (starting at 0).
//    logComment    in       Reason for update.
//    tableName     in       Table name (default: "" - get table name from object type)
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create Writer.


//  Program Notes:-
//  =============

//  None.


  DbiTrace(  "Creating ND::TDbiWriter for " << fTableName << "  ");

  Open(vr,aggNo,task,creationDate,dbNo,logComment);

}

//.....................................................................

template<class T>
ND::TDbiWriter<T>::TDbiWriter(const ND::TVldRange& vr,
                        Int_t aggNo,
                        TDbi::Task task,
                        ND::TVldTimeStamp creationDate,
                        const std::string& dbName,
                        const std::string& logComment,
                        const std::string& tableName) :
fAggregateNo(aggNo),
fPacket(new ND::TDbiSqlValPacket),
fRequireGlobalSeqno(0),
fTableProxy(&ND::TDbiWriter<T>::GetTableProxy(tableName)),
fTableName(fTableProxy->GetTableName()),
fUseOverlayCreationDate(creationDate == ND::TVldTimeStamp(0,0)),
fValidRec(0),
fLogEntry(fTableName,logComment,vr.GetDetectorMask(),vr.GetSimMask(),task)
{
//
//  Purpose:  Constructor
//
//  Arguments:
//    vr            in       Validity range of validity set.
//    aggNo         in       Aggregate number of validity set.
//    task          in       Task of validity set.
//    creationDate  in       Creation date of validity set.
//    dbName        in       Database name.
//    logComment    in       Reason for update.
//    tableName     in       Table name (default: "" - get table name from object type)
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create Writer.


//  Program Notes:-
//  =============

//  None.


  DbiTrace(  "Creating ND::TDbiWriter for " << fTableName << "  ");

  Open(vr,aggNo,task,creationDate,dbName,logComment);

}

//.....................................................................

template<class T>
ND::TDbiWriter<T>::TDbiWriter(const ND::TDbiValidityRec& vrec,
                        UInt_t dbNo,
                        const std::string& logComment) :
fAggregateNo(0),
fDbNo(dbNo),
fPacket(new ND::TDbiSqlValPacket),
fRequireGlobalSeqno(0),
fTableProxy(0),
fUseOverlayCreationDate(kFALSE),
fValidRec(new ND::TDbiValidityRec(vrec)),
fLogEntry(fTableName,logComment,vrec.GetVldRange().GetDetectorMask(),
          vrec.GetVldRange().GetSimMask(),vrec.GetTask())
{
//
//  Purpose:  Constructor
//
//  Arguments:
//    vrec          in       Validity record for set.
//                           MUST have an associated ND::TDbiTableProxy
//    dbNo          in       Database number in cascade (starting at 0).
//    logComment    in       Reason for update.
//
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create Writer.


//  Program Notes:-
//  =============

//  None.


  T pet;
  DbiTrace(  "Creating ND::TDbiWriter for " << pet.GetName() << "  ");

  this->Open(vrec,dbNo,logComment);

}

//.....................................................................

template<class T>
ND::TDbiWriter<T>::TDbiWriter(const ND::TDbiValidityRec& vrec,
                        const std::string& dbName,
                        const std::string& logComment) :
fAggregateNo(0),

fPacket(new ND::TDbiSqlValPacket),
fRequireGlobalSeqno(0),
fTableProxy(0),
fUseOverlayCreationDate(kFALSE),
fValidRec(new ND::TDbiValidityRec(vrec)),
fLogEntry(fTableName,logComment,vrec.GetVldRange().GetDetectorMask(),
          vrec.GetVldRange().GetSimMask(),vrec.GetTask())
{
//
//  Purpose:  Constructor
//
//  Arguments:
//    vrec          in       Validity record for set.
//                           MUST have an associated ND::TDbiTableProxy
//    dbName        in       Database name.
//    logComment    in       Reason for update.
//
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create Writer.


//  Program Notes:-
//  =============

//  None.


  T pet;
  DbiTrace(  "Creating ND::TDbiWriter for " << pet.GetName() << "  ");

  this->Open(vrec,dbName,logComment);

}

//.....................................................................

template<class T>
ND::TDbiWriter<T>::~TDbiWriter() {
//
//
//  Purpose: Destructor


  DbiTrace( "Destroying ND::TDbiWriter" << "  ");

  Reset();
  delete fPacket;
  fPacket = 0;
  delete fValidRec;
  fValidRec = 0;

}


//.....................................................................

template<class T>
ND::TDbiWriter<T>& ND::TDbiWriter<T>::operator<<(const T& row) {
//
//
//  Purpose:  Stream row into this ND::TDbiWriter.

  if ( ! this->IsOpen() ) return *this;

  Int_t AggNoRow =  row.GetAggregateNo();
  if (  fAggregateNo != AggNoRow ) {
    DbiSevere(  "Cannot store row data for table  "
      << fTableName
      << ", current set has aggregate no.: " << fAggregateNo
      << ", but it has: " << AggNoRow
      << " \n  closing ND::TDbiWriter" << "  ");
    this->Abort();
    return *this;
  }
  if ( ! fPacket->AddDataRow(*fTableProxy,fValidRec,row) ) {
    DbiSevere( "Closing ND::TDbiWriter due to above error." << "  ");
    this->Abort();
  }

  return *this;

}
//.....................................................................

template<class T>
UInt_t ND::TDbiWriter<T>::GetEpoch() const {
//
//
//  Purpose:  Return epoch of associated ND::TDbiValidityRec if any.

  return fValidRec ? fValidRec->GetEpoch() : 0;

}
//.....................................................................

template<class T>
Bool_t ND::TDbiWriter<T>::CanOutput(Bool_t reportErrors) const {
//
//
//  Purpose:  Return true if open and ready to receive/output  data.

  if ( ! this->IsOpen(reportErrors) ) return kFALSE;

  if ( this->NeedsLogEntry() && ! fLogEntry.HasReason() ) {
    if ( reportErrors ) DbiSevere(  "Cannot output validity set for table "
      << fTableName
      << ", writing to Master DB but no log comment has been supplied." << "  ");
    return kFALSE;
  }

  UInt_t nstmts = fPacket->GetNumSqlStmts();

  if ( nstmts == 0 ) {
    // not even a VLD insert
    if ( reportErrors ) DbiSevere(  "Cannot output validity set for table "
      << fTableName
      << ", no data has been written." << "  ");
    return kFALSE;
  }

  if ( nstmts == 1 ) {
    // if VLD insert present but no data rows, just issue a stern warning
    // as it may be intended to obscure now invalid data
    string stmt = fPacket->GetStmt(0);
    bool isvldentry =
      (stmt.find("INSERT INTO") != string::npos ) &&
      (stmt.find("VLD VALUES")  != string::npos);
    if ( isvldentry ) {
      if ( reportErrors ) DbiWarn(  "VLD entry but no data for "
        << fTableName << "  ");
    }
    else {
      // this should never, ever happen ... but let's be paranoid
      if ( reportErrors ) {
        DbiSevere(  "Cannot output validity set for table "
          << fTableName
          << ", (no VLD info) no data has been written." << "  ");
        fPacket->Print();
      }
      return kFALSE;
    }
  }

  return kTRUE;
}


//.....................................................................

template<class T>
Bool_t ND::TDbiWriter<T>::Close(const char* fileSpec) {
//
//
//  Purpose:  Close current validity set and write it to the database.
//
//  Arguments:
//
//  fileSpec       in   File pointer.  If not null write to file instead of database.
//
//  Return:    kTRUE is I/O successful.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Close current validity set and write it to the database.

//  Program Notes:-
//  =============

//  None.

  Bool_t ok = kFALSE;

// Skip output unless good data to output.

  if ( CanOutput() ) {

//  Find the next free sequence number.  It must be global if writing to a
//  to a file, otherwise it can be local if DB isn't authorising.
    int seqNoType = fileSpec ? 1 : fRequireGlobalSeqno;
    Int_t seqNo = fTableProxy->GetCascader().AllocateSeqNo(fTableName,seqNoType,fDbNo);
    if ( seqNo <= 0 ) {
      DbiSevere(  "Cannot get sequence number for table "
        << fTableName << "  ");
    }

    else {

//  Use overlay creation date if required.
      if ( fUseOverlayCreationDate &&  fValidRec
	  ) fPacket->SetCreationDate(fTableProxy->QueryOverlayCreationDate(*fValidRec,fDbNo));

//  Set SEQNO and perform I/O.
      fPacket->SetSeqNo(seqNo);
      if ( fileSpec ) {
        ofstream fs(fileSpec,ios_base::app);
        if ( ! fs ) DbiSevere( "Unable to open " << fileSpec << "  ");
        else        ok = fPacket->Write(fs,true);
      }
      else {
        ok = fPacket->Store(fDbNo);
      }

//  Record update if I/O successful and required.
      if ( ok && this->WritingToMaster() ) {
       if ( this->NeedsLogEntry() || fLogEntry.HasReason() )
	ok = fLogEntry.Write(fDbNo,seqNo);
      }
    }
  }

//  Clear out ready for any further I/O.

  fPacket->Clear();

  return ok;
}
//.....................................................................

template<class T>
void ND::TDbiWriter<T>::CompleteOpen(UInt_t dbNo,
                                const std::string& logComment) {
//
//
//  Purpose:  Complete the open process
//
//  Arguments:
//    dbNo          in       Database number in cascade (starting at 0).
//    logComment    in       Reason for update.
//
//
//  Specification:-
//  =============
//
//  o Complete reinitialisation for new I/O

//  Program Notes:-
//  =============

//  Assumes that fTableProxy, fUseOverlayCreationDate, fValidRec are
//  already initialised.


  fDbNo = dbNo;
  if ( (int) fDbNo < 0 ) {
    DbiSevere( "Bad database name/number selected. " << "  ");
    this->Reset();
    return;
  }

  fAggregateNo = fValidRec->GetAggregateNo();
  fTableName   = fTableProxy->GetTableName(),

// Recreate validity packet.
  fPacket->Recreate(fTableName,
                    fValidRec->GetVldRange(),
                    fAggregateNo,
                    fValidRec->GetTask(),
                    fValidRec->GetCreationDate());

// Recreate log entry.
  const ND::TVldRange&  vr = fValidRec->GetVldRange();
  fLogEntry.Recreate(fTableName,
                     logComment,
                     vr.GetDetectorMask(),
                     vr.GetSimMask(),
                     fValidRec->GetTask());

}

//.....................................................................

template<class T>
ND::TDbiTableProxy& ND::TDbiWriter<T>::GetTableProxy() {
//
//
//  Purpose:  Private static function to find default associated
//            ND::TDbiTableProxy.

//  Program Notes:-
//  =============

//  As ND::TDbiResultSetHandle classes are in 1:1 correspondance with ND::TDbiWriter
//  classes, ND::TDbiWriter uses the static methods from ND::TDbiResultSetHandle
//  to find the associated ND::TDbiTableProxy.

  return ND::TDbiResultSetHandle<T>::GetTableProxy();
}

//.....................................................................

template<class T>
ND::TDbiTableProxy& ND::TDbiWriter<T>::GetTableProxy(
                                           const std::string& tableName){
//
//
//  Purpose:  Private static function to find an alternative
//            associated ND::TDbiTableProxy.
//
//  Arguments:
//    tableName    in    Alternative table name
//
//  Return:   Associated ND::TDbiTableProxy.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Ask TDbiRegistry for alternative  Table Proxy and return it.

//  As ND::TDbiResultSetHandle classes are in 1:1 correspondance with ND::TDbiWriter
//  classes, ND::TDbiWriter uses the static methods from ND::TDbiResultSetHandle
//  to find the associated ND::TDbiTableProxy.

  return ND::TDbiResultSetHandle<T>::GetTableProxy(tableName);

}
//.....................................................................

template<class T>
Bool_t ND::TDbiWriter<T>::IsOpen(Bool_t reportErrors) const {
//
//
//  Purpose:  Return true if open and ready to receive data.

  if ( ! ND::TDbiDatabaseManager::IsActive()  ) {
    if ( reportErrors ) DbiSevere(  "Cannot use ND::TDbiWriter, the DBI has been shutdown." << "  ");
  }
  else if (    !fValidRec
            ||  fPacket->GetNumSqlStmts() < 1 ) {
    if ( reportErrors ) DbiSevere(  "Cannot do I/O on ND::TDbiWriter for "
      << fTableName
      <<", it is currently closed." << "  ");
  }
  else return kTRUE;

  return kFALSE;

}
//.....................................................................

template<class T>
Bool_t ND::TDbiWriter<T>::NeedsLogEntry() const {
//
//
//  Purpose:  Return true if a log entry is required.

  // Some tables are created automatically so don't require entries.

  // ND280 may never use ND::TDbiWriter but if it does, assume that table
  // updates won't require an associated  ND::TDbiLogEntry.

  return false;

  // All other tables need entries if writing to a Master database.

  return this->WritingToMaster();

}

//.....................................................................

template<class T>
Bool_t ND::TDbiWriter<T>::Open (const ND::TVldRange& vr,
                           Int_t aggNo,
                           TDbi::Task task,
                           ND::TVldTimeStamp creationDate,
                           UInt_t dbNo,
                           const std::string& logComment) {
//
//
//  Purpose:  Output any existing data and open new validity set.
//
//  Arguments:
//    vr            in       Validity range of validity set.
//    aggNo         in       Aggregate number of validity set.
//    task          in       Task of validity set.
//    creationDate  in       Creation date of validity set.
//    dbNo          in       Database number in cascade (starting at 0).
//    logComment    in       Reason for update.
//
//  Return:    kTRUE if no errors detected.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Output any existing data and open new validity set.

//  Program Notes:-
//  =============

//  None.

  bool ok = true;

  if ( ! ND::TDbiDatabaseManager::IsActive() ) return kFALSE;
  if ( this->CanOutput(kFALSE) ) ok = Close();

// Test for special creation date.
  fUseOverlayCreationDate = creationDate == ND::TVldTimeStamp(0,0);

//  Create a validity record.
  delete fValidRec;
  fValidRec = new ND::TDbiValidityRec(vr,task,aggNo,0,0,kFALSE,creationDate);

// Get a proxy.
  fTableProxy = &ND::TDbiWriter<T>::GetTableProxy(fTableName);

// Complete opening.
  this->CompleteOpen(dbNo,logComment);
  return ok;

}
//.....................................................................

template<class T>
Bool_t ND::TDbiWriter<T>::Open (const ND::TVldRange& vr,
                           Int_t aggNo,
                           TDbi::Task task,
                           ND::TVldTimeStamp creationDate,
                           const string& dbName,
                           const std::string& logComment) {
//
//
//  Purpose:  Output any existing data and open new validity set.
//
//  Arguments:
//    vr            in       Validity range of validity set.
//    aggNo         in       Aggregate number of validity set.
//    task          in       Task of validity set.
//    creationDate  in       Creation date of validity set.
//    dbName        in       Database name.
//    logComment    in       Reason for update.
//
//  Return:    kTRUE if no errors detected.

  this->SetDbName(dbName);
  return this->Open(vr,
                    aggNo,
                    task,
                    creationDate,
                    fDbNo,
                    logComment);

}

//.....................................................................

template<class T>
Bool_t ND::TDbiWriter<T>::Open(const ND::TDbiValidityRec& vrec,
                          UInt_t dbNo,
                          const std::string& logComment) {

//
//  Purpose:  Output any existing data and open new validity set.
//
//  Arguments:
//    vrec          in       Validity record for set.
//                           MUST have an associated ND::TDbiTableProxy
//    dbNo          in       Database number in cascade (starting at 0).
//    logComment    in       Reason for update.
//
//
//  Return:    kTRUE if no errors detected.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Output any existing data and open new validity set.


//  Program Notes:-
//  =============

//  None.

  bool ok = true;

  if ( ! ND::TDbiDatabaseManager::IsActive() ) return kFALSE;
  if ( this->CanOutput(kFALSE) ) ok = Close();


// Check that vrec is associated with the right table, but waive
// test for ND::TDbiConfigSet so that it can be used with any.
  const ND::TDbiTableProxy& proxyDefault = ND::TDbiWriter<T>::GetTableProxy();
  const ND::TDbiTableProxy& proxyVrec    = *vrec.GetTableProxy();
  if (    proxyDefault.GetTableName() != "DBICONFIGSET"
       &&    proxyVrec.GetTableName() != proxyDefault.GetTableName() ) {
    DbiSevere(  "Unable to create ND::TDbiWriter from query:" << "  "
       << vrec
       << " was filled by " << proxyVrec.GetTableName()
       << " not by " << proxyDefault.GetTableName() << "  ");
    this->Reset();
    return false;
  }
  else {
//  Have to cast away const - ND::TDbiWriter needs a modifiable ND::TDbiTableProxy.
    fTableProxy = const_cast<ND::TDbiTableProxy*>(&proxyVrec);
  }

  fUseOverlayCreationDate = vrec.GetCreationDate() == ND::TVldTimeStamp(0,0);

  delete fValidRec;
  fValidRec = new ND::TDbiValidityRec(vrec);

// Complete opening.
  this->CompleteOpen(dbNo,logComment);
  return ok;

}

//.....................................................................

template<class T>
Bool_t ND::TDbiWriter<T>::Open(const ND::TDbiValidityRec& vrec,
                          const string& dbName,
                          const std::string& logComment) {

//
//  Purpose:  Output any existing data and open new validity set.
//
//  Arguments:
//    vrec          in       Validity record for set.
//                           MUST have an associated ND::TDbiTableProxy
//    dbName        in       Database name.
//    logComment    in       Reason for update.
//
//
//  Return:    kTRUE if no errors detected.

  this->SetDbName(dbName);
  return this->Open(vrec,
                    fDbNo,
                    logComment);

}

//.....................................................................

template<class T>
void ND::TDbiWriter<T>::Reset() {
//
//
//  Purpose:  Clear out data and reset ready for any further I/O.
//
//  Arguments: None.
//
//  Return:    None.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Clear out data and reset ready for any further I/O.

//  Program Notes:-
//  =============

//  None.

  fAggregateNo = 0;
  fDbNo        = 0;
  fPacket->Clear();
  fTableProxy  = &ND::TDbiWriter<T>::GetTableProxy();
  fTableName   = fTableProxy->GetTableName();

  delete fValidRec;
  fValidRec    = 0;

}
//.....................................................................

template<class T>
void ND::TDbiWriter<T>::SetDbName(const string& dbName) {
//
//
//  Purpose:  Set Database entry by name.  Default: entry 0.

  if ( dbName == "" ) {
    fDbNo = 0;
  }
  else {
    fDbNo =  ND::TDbiDatabaseManager::Instance().GetCascader().GetDbNo(dbName);
  }
}
//.....................................................................

template<class T>
void ND::TDbiWriter<T>::SetEpoch(UInt_t epoch) {
//
//
//  Purpose:  Set epoch of associated ND::TDbiValidityRec if any.

  if ( fValidRec ) fValidRec->SetEpoch(epoch);
  fPacket->SetEpoch(epoch);

}

//.....................................................................

template<class T>
void ND::TDbiWriter<T>::SetLogComment(const std::string& reason) {
//
//
//  Purpose:  Set log comment.

  fLogEntry.SetReason(reason);

}

//.....................................................................

template<class T>
ND::TDbiTableProxy& ND::TDbiWriter<T>::TableProxy() const  {
//
//
//  Purpose:  Return ND::TDbiTableProxy.
//

  assert( ND::TDbiDatabaseManager::IsActive() );
  return *fTableProxy;
}

//.....................................................................

template<class T>
Bool_t ND::TDbiWriter<T>::WritingToMaster() const {
//
//
//  Purpose:  Return true if writing permanent data to a Master Database.


  ND::TDbiCascader& cascader = ND::TDbiDatabaseManager::Instance().GetCascader();
  return  (    fDbNo == (UInt_t) cascader.GetAuthorisingDbNo()
            && ! cascader.IsTemporaryTable(fTableName,fDbNo));

}

/*    Template for New Member Function

//.....................................................................

template<class T>
ND::TDbiWriter<T>:: {
//
//
//  Purpose:
//
//  Arguments:
//    xxxxxxxxx    in    yyyyyy
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o

//  Program Notes:-
//  =============

//  None.


}

*/


