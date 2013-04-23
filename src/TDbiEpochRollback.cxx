///////////////////////////////////////////////////////////////////////
// $Id: TDbiEpochRollback.cxx,v 1.2 2012/06/14 10:55:22 finch Exp $
//
// CP::TDbiEpochRollback

#include <cstring>
#include <sstream>

#include "TString.h"

#include "TDbi.hxx"
#include "TDbiEpochRollback.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>
#include "TDbiRegistry.hxx"
#include "UtilString.hxx"
#include "TVldTimeStamp.hxx"

ClassImp(CP::TDbiEpochRollback)

//   Definition of static data members
//   *********************************


// Definition of member functions (alphabetical order)
// ***************************************************


//.....................................................................

CP::TDbiEpochRollback::TDbiEpochRollback()
{
//
//
//  Purpose:  Default constructor
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create CP::TDbiEpochRollback.


//  Program Notes:-
//  =============

//  None.


    DbiTrace( "Creating CP::TDbiEpochRollback" << "  ");
}
//.....................................................................

CP::TDbiEpochRollback::~TDbiEpochRollback() {
//
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Destroy CP::TDbiEpochRollback.



  DbiTrace( "Destroying CP::TDbiEpochRollback" << "  ");

}
//.....................................................................

const std::string& CP::TDbiEpochRollback::GetEpochCondition(const std::string& tableName) const {
//
//
//  Purpose:  Return epoch rollback condition  associated with named table.
//            - returns an empty string if there is no associated condition
//
//  Contact:   N. West

//  Program Notes:-
//  =============

//  Search in reverse order so that specific entries are processed
//  before generic (i.e. ones that end in wildcard *).

  static std::string condition;

  condition = "";

  name_map_t::const_reverse_iterator itr    = fTableToEpoch.rbegin();
  name_map_t::const_reverse_iterator itrEnd = fTableToEpoch.rend();

  for (; itr != itrEnd; ++itr) {
    if ( ! CP::UtilString::cmp_wildcard(tableName,itr->first) ) {
    	std::ostringstream condition_stream;
    	condition_stream << "EPOCH <= ";
    	condition_stream << itr->second;
      condition = condition_stream.str();
      return condition;
    }
  }
  return condition;
}

//.....................................................................

void CP::TDbiEpochRollback::Set(TDbiRegistry& reg) {
//
//
//  Purpose:  Extract epoch Rollback from TDbiRegistry.
//
//  Arguments:
//    reg          in    TDbiRegistry containing "EpochRollback:"
//                 out   Updated TDbiRegistry with this key removed.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Extract epoch Rollback from TDbiRegistry.

  TDbiRegistry::TDbiRegistryKey keyItr(&reg);

  Bool_t  hasChanged = kFALSE;

  const char* key = keyItr();
  while ( key ) {

    const char* nextKey =  keyItr();

    // Process EpochRollback keys

    if ( ! strncmp("EpochRollback:",key,14) ) {
      std::string tableName = key+14;
      Int_t epoch;
      bool  ok = reg.Get(key,epoch);
      if ( ok ) {
        fTableToEpoch[tableName] = epoch;
        hasChanged = kTRUE;

      }
      else DbiWarn(  "Illegal EpochRollback registry item: " << key
	<< " = " <<  reg.GetValueAsString(key) << "  ");
      reg.RemoveKey(key);
    }
    key = nextKey;
  }

  if ( hasChanged ) this->Show();
}
//.....................................................................

void CP::TDbiEpochRollback::Show() const {
//
//
//  Purpose:  Print out the current EpochRollback status.
//
//
//  Contact:   N. West
//

 std::ostream& msg=TDbiLog::GetLogStream();
  msg << "\n\nEpochRollback Status:  ";
  if ( fTableToEpoch.size() == 0 ) msg <<"Not enabled" << std::endl;
  else {
      msg << "Maximum EPOCH limits:- " << std::endl;
    name_map_t::const_reverse_iterator itr    = fTableToEpoch.rbegin();
    name_map_t::const_reverse_iterator itrEnd = fTableToEpoch.rend();
    for (; itr != itrEnd; ++itr) {
      std::string name = itr->first;
      if ( name.size() < 30 ) name.append(30-name.size(),' ');
      msg <<"    " << name << "  " << itr->second << std::endl;
    }
  }
}



