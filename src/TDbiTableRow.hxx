#ifndef TDBITABLEROW_H
#define TDBITABLEROW_H

//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 *
 * $Id: TDbiTableRow.hxx,v 1.2 2011/06/09 14:44:29 finch Exp $
 *
 * \class CP::TDbiTableRow
 *
 *
 * \brief
 * <b>Concept</b>  Abstract interface to TableRow objects.  Each database
 *  table consists of a collection of TableRow objects.
 *
 * \brief
 * <b>Purpose</b> Provides a generic interface to TableRow objects permiting
 *   much of the database machinery to be generic.
 *
 * \brief
 * <b>Subclass CP::obligations</b> All subclasses must have public default
 *  and constructors.
 *
 * \brief
 * <b>Program Notes</b>  It inherits from TObject only so that it can get
 *   the class name at execution time.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include "TObject.h"
#include <string>
#include <cassert>

namespace CP {
class TDbiOutRowStream;
class TDbiResultSet;
class TDbiInRowStream;
class TDbiTableProxy;
class TDbiValidityRec;
}

namespace CP {
class TDbiTableRow : public TObject
{

public:

// Constructors and destructors.
           TDbiTableRow();
           TDbiTableRow(const TDbiTableRow& from);
  virtual ~TDbiTableRow();

// State testing member functions
/// Replace this with a function returning true in order to use the level 2 disk cache.
virtual       Bool_t CanL2Cache() const { return kFALSE; } 
//virtual       Bool_t CanL2Cache() const { return kTRUE; } //FOR TESTS

virtual        Int_t GetAggregateNo() const { return -1; }
          TDbiResultSet* GetOwner() const { return fOwner; }
virtual TDbiTableRow* CreateTableRow() const =0;
virtual       UInt_t GetIndex(UInt_t defIndex) const {return defIndex;}

// State modifying member functions
          void SetOwner(TDbiResultSet* owner) {fOwner = owner;}

// I/O  member functions
  virtual void Fill(TDbiInRowStream& rs,
                    const TDbiValidityRec* vrec) =0;
  virtual void Store(TDbiOutRowStream& /* ors */,
                     const TDbiValidityRec* /* vrec */) const { assert(0); }

private:

// Data members

/// The owning TDbiResultSet, if any.
  TDbiResultSet * fOwner;

ClassDef(TDbiTableRow,0)   // TDbiTableRow for a specific database table.

};
};


#endif  // TDBITABLEROW_H

