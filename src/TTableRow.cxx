#include "TTableRow.hxx"

ClassImp(ND::TTableRow);

//_____________________________________________________________________________

ND::TTableRow::TTableRow()  {

   // Default constructor

    DbiTrace("ND::TTableRow: Default ctor at:" << (void*) this);

}


//_____________________________________________________________________________

ND::TTableRow::TTableRow(const ND::TTableRow& from) {

   // Copy constructor

    DbiTrace("ND::TTableRow: Copy ctor at:" << (void*) this);

    //  Nothing to copy; no data member in ND::TDbiTableRow.

}

//_____________________________________________________________________________

ND::TTableRow::~TTableRow()  {

   // Destructor

    DbiTrace("ND::TTableRow: dtor at:" << (void*) this);

}

