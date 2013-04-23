#include "TDemo_DB_Table.hxx"
#include "TResultInputStream.hxx"
#include "TDbiLog.hxx"

ClassImp(ND::TDemo_DB_Table);



//  Instantiate associated ND::TResultSetHandle and ND::TDbiResultSetHandle classes.
//  *****************************************************************

// The TableRow subclass' *_.cxx must request the instantiation of the
// templated ND::TResultSetHandle class it as the supplied class.  However
// ND::TResultSetHandle inherits from the backend ND::TDbiResultSetHandle so an
// instatiation of it is also required.

#include "TResultSetHandle.cxx"
template class  ND::TResultSetHandle<ND::TDemo_DB_Table>;
template class  ND::TDbiResultSetHandle<ND::TDemo_DB_Table>;


//_____________________________________________________________________________

ND::TDemo_DB_Table::TDemo_DB_Table()  {

   // Default constructor

    DbiTrace("ND::TDemo_DB_Table: Default ctor at:" << (void*) this);

}


//_____________________________________________________________________________

ND::TDemo_DB_Table::TDemo_DB_Table(const ND::TDemo_DB_Table& from) {

   // Copy constructor

    DbiTrace("ND::TDemo_DB_Table: Copy ctor at:" << (void*) this);

    *this = from;

}

//_____________________________________________________________________________

ND::TDemo_DB_Table::~TDemo_DB_Table()  {

   // Destructor

    DbiTrace("ND::TDemo_DB_Table: dtor at:" << (void*) this);

}

//_____________________________________________________________________________

void ND::TDemo_DB_Table::Fill(ND::TResultInputStream& ris) {

/// This method is used to fill a row.  A ResultInputStream is an object
/// with operator >> methods for most basic types.
///
/// Individual data items are read from it in the order in which they
/// appear in the database table but notice that the first two rows (SEQNO
/// and ROW_COUNTER) which are not part of the row's state, do not have to
/// be skipped.
///
/// Also notice how the row state can include objects even though the
/// stored state can only store basic types. E_CHAN_ID is stored as a
/// MySQL INT but is used to fill a TTFBChannelId.


  UInt_t id;
  ris >> id >> fIParm1 >> fIParm2 >> fIParm3 >> fFParm1 >> fFParm2 >> fFParm3;
  fChannelId = TTFBChannelId(id);

}

//_____________________________________________________________________________

void ND::TDemo_DB_Table::Print(const Option_t*) const {

  // Programming note: The arg isn't used but is there so as to avoid compiler
  // warnings about hidden method TObject::Print(const Option_t*).

  DbiLog("DEMO_DB_TABLE row for channel " << fChannelId.AsString()
	   << " integer parms: "       << fIParm1 << "," << fIParm2 << "," << fIParm3
	   << " floating point parms " << fFParm1 << "," << fFParm2 << "," << fFParm3 );

}

