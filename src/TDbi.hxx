// $Id: TDbi.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $

#ifndef TDbi_hxx_seenXYZDBI_H
#define TDbi_hxx_seenXYZDBI_H

#include "TVldContext.hxx"
#include "TVldTimeStamp.hxx"

#include <ECore.hxx>

#include <map>
#include <string>

namespace CP {
    EXCEPTION(EOfflineDatabase,ECore);
    EXCEPTION(ENoEnvironment,EOfflineDatabase);
    EXCEPTION(EBadDatabase,EOfflineDatabase);
    EXCEPTION(EQueryFailed,EOfflineDatabase);
    EXCEPTION(EBadConnection,EOfflineDatabase);
    EXCEPTION(EBadTDbiRegistryKeys,EOfflineDatabase);
};

class TList;

///
/// \brief
/// <b>Concept</b> Package-wide namespace of utilities  and typedef/enums.
///
/// <b>Purpose</b> To provide standard  of utilities  and typedef/enums.
///
/// Contact: A.Finch@lancaster.ac.uk
///
namespace TDbi {

// Types and enum

    typedef  Int_t Task;

    enum TaskTypes {  kAnyTask     = -1,  // disable task selection in context queries.
                      kDefaultTask = 0
                   };

    enum TypeRegimes { kRootRegime = 0,
                       kSQLRegime  = 1
                     };

    enum DataTypes { kUnknown,
                     kBool,      // concept
                     kChar,      // concept
                     kUChar,     // concept
                     kTiny,
                     kUTiny,
                     kShort,
                     kUShort,
                     kInt,       // concept
                     kUInt,      // concept
                     kLong,
                     kULong,
                     kFloat,     // concept
                     kDouble,
                     kString,    // concept
                     kTString,
                     kDate       // concept
                   };


    enum DbTypes {  kMySQL         = 0
                 };

    enum AbortTest { kDisabled,
                     kTableMissing,
                     kDataMissing
                   };

    enum { kMAXLOCALSEQNO   = 99999999,    // Max local SeqNo.
           kMAXTABLENAMELEN = 80
         };         // Max length of table name.

    /// Return SQL needed to create VLD table.
    std::string GetVldDescr(const char* tableName,
                            Bool_t isTemporary = false);
    void SetLogLevel(int level);


    ///  Time gates for each table in secs. Used to trim validity searches.
    Int_t GetTimeGate(const std::string& tableName);
    void SetTimeGate(const std::string& tableName, Int_t timeGate);


    /// DateTime conversion utilities.
    std::string MakeDateTimeString(const CP::TVldTimeStamp& timeStamp);
    CP::TVldTimeStamp MakeTimeStamp(const std::string& sqlDateTime,
                                    Bool_t* ok =0);

    /// SeqNo utilities
    Bool_t NotGlobalSeqNo(UInt_t seqNo);
}
#endif

