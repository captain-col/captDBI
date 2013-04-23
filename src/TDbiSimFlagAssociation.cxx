// $Id: TDbiSimFlagAssociation.cxx,v 1.1 2011/01/18 05:49:20 finch Exp $
////////////////////////////////////////////////////////////////////////
// ND::TDbiSimFlagAssociation                                              //
//                                                                    //
// Package: ND::TDbi (Database Interface).                                 //
//                                                                    //
// N. West 08/2003                                                    //
//                                                                    //
// Concept: Association of a particular SimFlag type with a list of   //
//          SimFlag types.                                            //
//                                                                    //
// Purpose: To allow the DBI to choose an alternative SimFlag type    //
//          when attempting to satisfy queries.  For example, allow   //
//          MC data to use Data constants.                            //
//                                                                    //
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
using std::ostringstream;

#include <string>
using std::string;
#include <vector>
using std::vector;

#include "TDbiSimFlagAssociation.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
#include "TDbiRegistry.hxx"
#include "UtilString.hxx"

ClassImp(ND::TDbiSimFlagAssociation)


//   Definition of static data members
//   *********************************


const ND::TDbiSimFlagAssociation* ND::TDbiSimFlagAssociation::fgInstance = 0;

// Definition of global functions (alphabetical order)
// ***************************************************

ostream& operator<<(ostream& s, const ND::TDbiSimFlagAssociation& simFlagAss) {
  simFlagAss.Print(s);
  return s;
}

// Definition of member functions (alphabetical order)
// ***************************************************

//.....................................................................

ND::TDbiSimFlagAssociation::TDbiSimFlagAssociation() {
//
//
//  Purpose:  Default constructor
//
//  Contact:   N. West
//


  DbiTrace( "Creating ND::TDbiSimFlagAssociation" << "  ");

  // Connect to global pointer;
  fgInstance = this;

}


//.....................................................................

ND::TDbiSimFlagAssociation::~TDbiSimFlagAssociation() {
//
//
//  Purpose: Destructor
//
//  Contact:   N. West
//


  DbiTrace( "Destroying ND::TDbiSimFlagAssociation" << "  ");

  // Disconnect from global pointer;
  if ( fgInstance == this ) fgInstance = 0;

}
//.....................................................................

ND::TDbiSimFlagAssociation::SimList_t
     ND::TDbiSimFlagAssociation::Get(const ND::DbiSimFlag::SimFlag_t value)const {
//
//
//  Purpose:  Return associated list
//            or just list containing value if none.
//

  SimMap_t::const_iterator itr = fAssociations.find(value);
  if ( itr != fAssociations.end() ) return itr->second;
  SimList_t l;
  l.push_back(value);
  return l;

}

//.....................................................................

const ND::TDbiSimFlagAssociation& ND::TDbiSimFlagAssociation::Instance() {
//
//
//  Purpose:  Get access to the one and only instance.
//

//  Program Notes:-
//  =============

//  If necessary, creates a ND::TDbiSimFlagAssociation, but once
//  TDbiDatabaseManager.hxxas been created, it's owned version
//  will supersede it and orginal will be lost (leak).
//  In practice this should never happen; ND::TDbiDatabaseManager is
//  the first significant object to be created.

  if ( ! fgInstance ) new ND::TDbiSimFlagAssociation;
  // The act of creation will set fgInstance.
  return *fgInstance;

}
//.....................................................................

void ND::TDbiSimFlagAssociation::Print(ostream& s)const {
//
//
//  Purpose:  Print self.

  s << "\n\nSimFlag Association Status:  ";
  if ( fAssociations.size() == 0 ) s <<"Not enabled" << endl;
  else {
    s << endl;

    SimMap_t::const_iterator mapItr    = fAssociations.begin();
    SimMap_t::const_iterator mapItrEnd = fAssociations.end();
    while ( mapItr != mapItrEnd ) {

      ND::DbiSimFlag::SimFlag_t value = mapItr->first;
      string name = ND::DbiSimFlag::AsString(value);
      ostringstream buff;
      buff << name << "(" << value << ")";
      name = buff.str();
      if ( name.size() < 20 ) name.append(20-name.size(),' ');
      s << name << "maps to: ";

      SimList_t l = mapItr->second;
      SimList_t::const_iterator listItr    = l.begin();
      SimList_t::const_iterator listItrEnd = l.end();
      while ( listItr != listItrEnd ) {
        ND::DbiSimFlag::SimFlag_t v = *listItr;
        string n = ND::DbiSimFlag::AsString(v);
        s << n << "(" << v << ")";
	++listItr;
        if ( listItr != listItrEnd ) s << ", ";
      }
      s << endl;
      ++mapItr;
    }

  }
}

//.....................................................................

void ND::TDbiSimFlagAssociation::Set(TDbiRegistry& reg) {
//
//
//  Purpose:  Extract SimFlag association lists from TDbiRegistry.
//
//  Arguments:
//    reg          in    TDbiRegistry containing "SimFlagAssociation:..." keys.
//                 out   Updated TDbiRegistry with these keys removed.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Extract SimFlag association lists from TDbiRegistry.

  TDbiRegistry::TDbiRegistryKey keyItr(&reg);

  Bool_t  hasChanged = kFALSE;

  const char* key = keyItr();
  while ( key ) {

    const char* nextKey =  keyItr();
    if ( ! strncmp("SimFlagAssociation:",key,19) ) {

      // Found a SimFlagAssociation key, extract its value.
      string Name = key+19;
      ND::DbiSimFlag::SimFlag_t value = ND::DbiSimFlag::StringToEnum(Name.c_str());
      const char* listChars = 0;
      bool ok = reg.Get(key,listChars) && (value != ND::DbiSimFlag::kUnknown);
      // Collect the associated list
      SimList_t lv;
      if ( ok ) {
        vector<string> ls;
	ND::UtilString::StringTok(ls,listChars,",");
        vector<string>::iterator itr    = ls.begin();
        vector<string>::iterator itrEnd = ls.end();
        for (; itr != itrEnd; ++itr ) {
          ND::DbiSimFlag::SimFlag_t v = ND::DbiSimFlag::StringToEnum(itr->c_str());
          if ( v == ND::DbiSimFlag::kUnknown) ok = false;
          lv.push_back(v);
	}
      }

      if ( ok ) {
        this->Set(value,lv);
	hasChanged = true;
      }
      else  DbiWarn(  "Illegal SimFlagAssociation registry item: " << key
	  << " = " << listChars << "  ");

      reg.RemoveKey(key);
    }
    key = nextKey;
  }

  if ( hasChanged ) this->Show();
}

//.....................................................................

void ND::TDbiSimFlagAssociation::Show() {
//
//
//  Purpose:

  DbiInfo( *this << "  ");


}

/*    Template for New Member Function

//.....................................................................

ND::TDbiSimFlagAssociation:: {
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


