// $Id: TDbiConfigStream.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $

#include <sstream>



#include "TDbiConfigStream.hxx"
#include "TDbiFieldType.hxx"
#include "TDbiTableProxy.hxx"
#include "TDbiDatabaseManager.hxx"
#include "TDbiWriter.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>
#include "TDbiRegistry.hxx"
#include "TDbiCfg.hxx"
#include "TDbiCascader.hxx"

ClassImp(CP::TDbiConfigStream)

//   Definition of static data members
//   *********************************


CP::TVldContext  CP::TDbiConfigStream::fgDefaultContext(
    CP::DbiDetector::kDefault,
    CP::DbiSimFlag::kData,
    CP::TVldTimeStamp());

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................

CP::TDbiConfigStream::TDbiConfigStream() :
    fCFSet(0) {
//
//
//  Purpose:  Default constructor
//
//  Arguments: None.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create dummy ConfigStream.


//  Program Notes:-
//  =============

//  None.


    DbiDebug("Creating CP::TDbiConfigStream" << "  ");
}

//.....................................................................

CP::TDbiConfigStream::TDbiConfigStream(const std::string& SoftName,
                                       const std::string& ConfigName,
                                       CP::TVldContext vc,
                                       TDbi::Task task,
                                       const std::string& tableName) :
    fCFSet(0),
    fCFSetTable(tableName,vc,task),
    fConfigName(ConfigName),
    fSoftwName(SoftName) {
//
//
//  Purpose:  Standard constructor
//
//  Arguments:
//    SoftwName    in   Name of the software system to be configured
//    ConfigName   in   Name of the configuration set.
//                        Default: "default"
//    vc           in   Context.
//                        Default: TDbi::fgDefaultContext.
//    task         in   The task of the configuration.
//                        Default: 0
//    tableName    in   Name of configuration data table.
//                        Default: "SOFTWARE_CONFIGURATION"
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ConfigStream from specified table.


//  Program Notes:-
//  =============

//  None.


    DbiDebug("Creating CP::TDbiConfigStream" << "  ");

    // Search for row matching software and configuration names.
    int rowNum = fCFSetTable.GetNumRows()-1;
    while (rowNum >= 0) {
        fCFSet = fCFSetTable.GetRow(rowNum);
        if (fCFSet->GetParamValue(0) == fSoftwName
            && fCFSet->GetParamValue(1) == fConfigName) {
            fVRec = *fCFSetTable.GetValidityRec(fCFSet);
            DbiLog("CP::TDbiConfigStream for " << fSoftwName
                   << "," << fConfigName
                   << " has validity rec: " << fVRec
                   << " and aggregate no.: " << fCFSet->GetAggregateNo()
                   << "  ");
            return;
        }
        --rowNum;
    }

    // Cannot find matching row, leave configuration data as null
    // and set up a validity rec that can be used if creating a
    // new row.

    fCFSet = 0;

    fVRec.SetDbNo(0);
    fVRec.SetTableProxy(&CP::TDbiResultSetHandle<CP::TDbiConfigSet>::GetTableProxy(tableName));
    CP::TVldTimeStamp start(1970,1,1,0,0,0);
    CP::TVldTimeStamp   end(2038,1,1,0,0,0);
    CP::TVldRange vr(127,127,start,end,"CP::TDbiConfigStream");
    fVRec.SetVldRange(vr);
    DbiLog("CP::TDbiConfigStream for " << fSoftwName
           << "," << fConfigName
           << " has no existing entry; creating validity rec: " << fVRec << "  ");

}

//.....................................................................

CP::TDbiConfigStream::~TDbiConfigStream() {
//
//
//  Purpose: Destructor
//
//  Arguments:
//    None.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Destroy ConfigStream.


//  Program Notes:-
//  =============

//  None.


    DbiDebug("Destroying CP::TDbiConfigStream" << "  ");

}

//.....................................................................


std::ostream& CP::operator<<(std::ostream& os,
                             const CP::TDbiConfigStream& cfStream) {

//
//
//  Purpose:  Output status of CP::TDbiConfigStream on ostream.
//
//  Arguments:
//    os           in    ostream to be outout
//    cfStream     in    CP::TDbiConfigStream whise status is to be output.
//
//  Return:    Updated ostream
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Output status of CP::TDbiConfigStream on ostream.

//  Program Notes:-
//  =============

//  None.

    const CP::TDbiConfigSet* cfSet = cfStream.GetConfigSet();

    if (cfSet) {
        os << "CP::TDbiConfigSet contains: " << *cfSet << std::endl;
    }
    else {
        os << "CP::TDbiConfigSet is empty! " << std::endl;
    }
    return os;
}

//.....................................................................

const CP::TDbiConfigStream& CP::TDbiConfigStream::operator>>(TDbiRegistry* reg) {
//
//
//  Purpose:  Stream configuration data into TDbiRegistry object.
//
//  Arguments:
//    reg          in    Empty TDbiRegistry object (contents ignored)
//                 out   Filled TDbiRegistry object.
//
//  Return:    Original CP::TDbiConfigStream object.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Erase the contents of reg and refill from the CP::TDbiConfigSet
//    owned by this CP::TDbiConfigStream.

//  Program Notes:-
//  =============

//  None.

    if (! reg) {
        return *this;
    }

//  Record the current state of reg and then clear it.

    Bool_t keysLocked   = reg->KeysLocked();
    Bool_t valuesLocked = reg->ValuesLocked();
    reg->UnLockKeys();
    reg->UnLockValues();
    reg->Clear();

//  Use the owned CP::TDbiConfigSet to fill reg.

    if (fCFSet) {
        UInt_t numParams = fCFSet->GetNumParams();

//  Handle configuration tables.

        if (numParams == 3 && fCFSet->GetParamName(2) == "CONFIG_DATA") {
            std::istringstream is(fCFSet->GetParamValue(2));
            reg->ReadStream(is);
        }
        else {
            DbiSevere("Attempting to fill TDbiRegistry  from a table with "
                      << numParams << " columns (should be 3) using column named "
                      << fCFSet->GetParamName(2) << " (should be CONFIG_DATA)." << "  ");
        }
    }
    if (keysLocked) {
        reg->LockKeys();
    }
    if (valuesLocked) {
        reg->LockValues();
    }
    return *this;
}
//.....................................................................

CP::TDbiConfigStream& CP::TDbiConfigStream::operator<<(const TDbiRegistry* reg) {
//
//
//  Purpose:  Stream configuration data from TDbiRegistry object.
//
//  Arguments:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Refill internal fCFSetModified from reg..

//  Program Notes:-
//  =============

//  This does NOT write to the database.  To do that first use this
//  method to refill the configuration and then call the Write method.

    if (fSoftwName == "") {
        DbiSevere("Cannot fill (<<): No software name defined." << "  ");
        return *this;
    }

    CP::TDbiFieldType stringType(TDbi::kString);

    std::ostringstream os;
    reg->PrintStream(os);
    fCFSetModified.Clear();
    fCFSetModified.PushBack("SOFTW_NAME", fSoftwName,  stringType);
    fCFSetModified.PushBack("CONFIG_NAME",fConfigName, stringType);
    fCFSetModified.PushBack("CONFIG_DATA",os.str(),    stringType);
    if (fVRec.GetAggregateNo() > 0) {
        fCFSetModified.SetAggregateNo(fVRec.GetAggregateNo());
    }
    fCFSet =  &fCFSetModified;
    return *this;

}


//.....................................................................

Bool_t CP::TDbiConfigStream::Write(UInt_t dbNo,
                                   const std::string& logComment,
                                   Bool_t localTest) {
//
//
//  Purpose:  Write configuration data to the database.
//
//  Arguments:
//    dbNo         in    Database number in cascade (starting at 0).
//                         Default: 0.
//    logComment   in    Reason for update.
//                         Default: "".
//    localTest    in    Set true to use local SEQNOs (doesn't require authorising DB).
//                         Default: false.
//
//  Return:        True if I/O successful.

    if (! fCFSet) {
        DbiSevere("No configuration data to write out." << "  ");
        return false;
    }

    // If no aggregate number has been asigned so far, but fCFSet non-zero, then must
    // be creating a new software/config combination with the data in fCFSetModified.
    // Use a global seqno number (or local if localTest) to define a unique aggregate number.
    int requireGlobal = localTest ? -1 : 1;
    if (fVRec.GetAggregateNo() < 0) {
        CP::TDbiCascader& cas = CP::TDbiDatabaseManager::Instance().GetCascader();
        Int_t aggNo = cas.AllocateSeqNo(fVRec.GetTableProxy()->GetTableName(),requireGlobal,dbNo);
        if (aggNo <= TDbi::kMAXLOCALSEQNO && ! localTest) {
            DbiSevere("Cannot write out configuration data: no authorising entry in cascade." << "  ");
            return false;
        }
        fVRec.SetAggregateNo(aggNo);
        fCFSetModified.SetAggregateNo(aggNo);
        DbiDebug("Aggregate number: " << aggNo
                 << " allocated to entry " << fSoftwName
                 << "," << fConfigName
                 << " in table " << fVRec.GetTableProxy()->GetTableName() << "  ");
    }
    CP::TDbiWriter<CP::TDbiConfigSet> writer(fVRec,dbNo,logComment);
    writer.SetRequireGlobalSeqno(requireGlobal);
    writer.SetOverlayCreationDate();
    writer << *fCFSet;
    return writer.Close();

}



