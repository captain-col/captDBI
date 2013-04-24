///////////////////////////////////////////////////////////////////////
// $Id: TDbiRollbackDates.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
//
// CP::TDbiRollbackDates

#include <cstring>

#include "TString.h"

#include "TDbi.hxx"
#include "TDbiRollbackDates.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>
#include "TDbiRegistry.hxx"
#include "UtilString.hxx"
#include "TVldTimeStamp.hxx"

ClassImp(CP::TDbiRollbackDates)

//   Definition of static data members
//   *********************************


// Definition of member functions (alphabetical order)
// ***************************************************


//.....................................................................

CP::TDbiRollbackDates::TDbiRollbackDates() {
//
//
//  Purpose:  Default constructor
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create CP::TDbiRollbackDates.


//  Program Notes:-
//  =============

//  None.


    DbiTrace("Creating CP::TDbiRollbackDates" << "  ");
}
//.....................................................................

CP::TDbiRollbackDates::~TDbiRollbackDates() {
//
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Destroy CP::TDbiRollbackDates.



    DbiTrace("Destroying CP::TDbiRollbackDates" << "  ");

}
//.....................................................................

const std::string& CP::TDbiRollbackDates::GetDate(const std::string& tableName) const {
//
//
//  Purpose:  Return rollback date associated with named table.
//
//  Contact:   N. West

//  Program Notes:-
//  =============

//  Search in reverse order so that specific entries are processed
//  before generic (i.e. ones that end in wildcard *).

    static std::string date;

    name_map_t::const_reverse_iterator itr    = fTableToDate.rbegin();
    name_map_t::const_reverse_iterator itrEnd = fTableToDate.rend();
    for (; itr != itrEnd; ++itr)
        if (! CP::UtilString::cmp_wildcard(tableName,itr->first)
           ) {
            return itr->second;
        }
    return date;
}
//.....................................................................

const std::string& CP::TDbiRollbackDates::GetType(const std::string& tableName) const {
//
//
//  Purpose:  Return rollback time type associated with named table.
//
//  Contact:   N. West

//  Program Notes:-
//  =============

//  Search in reverse order so that specific entries are processed
//  before generic (i.e. ones that end in wildcard *).

    static std::string type("INSERTDATE");  // The default type

    name_map_t::const_reverse_iterator itr    = fTableToType.rbegin();
    name_map_t::const_reverse_iterator itrEnd = fTableToType.rend();
    for (; itr != itrEnd; ++itr)
        if (! CP::UtilString::cmp_wildcard(tableName,itr->first)
           ) {
            return itr->second;
        }
    return type;
}
//.....................................................................

void CP::TDbiRollbackDates::Set(TDbiRegistry& reg) {
//
//
//  Purpose:  Extract Rollback dates from TDbiRegistry.
//
//  Arguments:
//    reg          in    TDbiRegistry containing "Rollback:" and "RollbackType:" keys.
//                 out   Updated TDbiRegistry with these keys removed.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Extract Rollback dates from TDbiRegistry.

    TDbiRegistry::TDbiRegistryKey keyItr(&reg);

    Bool_t  hasChanged = kFALSE;

    const char* key = keyItr();
    while (key) {

        const char* nextKey =  keyItr();

        // Process Rollback keys

        if (! strncmp("Rollback:",key,9)) {
            std::string tableName = key+9;
            std::string date;
            const char* dateChars = 0;
            bool  ok = reg.Get(key,dateChars);
            if (ok) {
                date = dateChars;
                TVldTimeStamp ts(TDbi::MakeTimeStamp(date,&ok));
                date = TDbi::MakeDateTimeString(ts);
            }
            if (ok) {

                // Prune away any trailing spaces - they cause SQL
                // to fail expressions involving the date.
                int loc = date.size()-1;
                while (loc && date[loc] == ' ') {
                    date.erase(loc--);
                }

                fTableToDate[tableName] = date;
                hasChanged = kTRUE;

            }
            else DbiWarn("Illegal Rollback registry item: " << key
                             << " = " << dateChars << "  ");
            reg.RemoveKey(key);
        }

        // Process RollbackType keys

        else if (! strncmp("RollbackType:",key,13)) {
            std::string tableName = key+13;
            TString type;
            const char* typeChars = 0;
            bool  ok = reg.Get(key,typeChars);
            if (ok) {
                // Convert to upper case and remove any leading or trailing spaces
                type = typeChars;
                type.ToUpper();
                type = type.Strip(TString::kBoth);
                ok = ! type.CompareTo("INSERTDATE") || ! type.CompareTo("CREATIONDATE");
            }
            if (ok) {
                fTableToType[tableName] = type.Data();
                hasChanged = kTRUE;
            }
            else DbiWarn("Illegal RollbackType registry item: " << key
                             << " = " << typeChars << "  ");
            reg.RemoveKey(key);
        }
        key = nextKey;
    }

    if (hasChanged) {
        this->Show();
    }
}
//.....................................................................

void CP::TDbiRollbackDates::Show() const {
//
//
//  Purpose:  Print out the current Rollback date status.
//
//
//  Contact:   N. West
//

    std::ostream& msg=TDbiLog::GetLogStream();
    msg << "\n\nRollback Status:  ";
    if (fTableToDate.size() == 0) {
        msg <<"Not enabled" << std::endl;
    }
    else {
        msg << "\n\n  Dates:- " << std::endl;
        name_map_t::const_reverse_iterator itr    = fTableToDate.rbegin();
        name_map_t::const_reverse_iterator itrEnd = fTableToDate.rend();
        for (; itr != itrEnd; ++itr) {
            std::string name = itr->first;
            if (name.size() < 30) {
                name.append(30-name.size(),' ');
            }
            msg <<"    " << name << "  " << itr->second << std::endl;
        }
        msg << "\n  Rollback Type is 'INSERTDATE'";
        if (fTableToType.size()) {
            msg << " except as follows:- " << std::endl;
            itr    = fTableToType.rbegin();
            itrEnd = fTableToType.rend();
            for (; itr != itrEnd; ++itr) {
                std::string name = itr->first;
                if (name.size() < 30) {
                    name.append(30-name.size(),' ');
                }
                msg <<"    " << name << "  " << itr->second << std::endl;
            }
        }
        msg << std::endl;
    }
}


/*    Template for New Member Function

//.....................................................................

CP::TDbiRollbackDates:: {
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


