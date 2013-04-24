#include "TTableRow.hxx"

ClassImp(CP::TTableRow);

//_____________________________________________________________________________

CP::TTableRow::TTableRow()  {

    // Default constructor

    DbiTrace("CP::TTableRow: Default ctor at:" << (void*) this);

}


//_____________________________________________________________________________

CP::TTableRow::TTableRow(const CP::TTableRow& from) {

    // Copy constructor

    DbiTrace("CP::TTableRow: Copy ctor at:" << (void*) this);

    //  Nothing to copy; no data member in CP::TDbiTableRow.

}

//_____________________________________________________________________________

CP::TTableRow::~TTableRow()  {

    // Destructor

    DbiTrace("CP::TTableRow: dtor at:" << (void*) this);

}

