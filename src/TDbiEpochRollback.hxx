#ifndef DBIEPOCHROLLBACK_H
#define DBIEPOCHROLLBACK_H

/**
 *
 * $Id: TDbiEpochRollback.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
 *
 * \class CP::TDbiEpochRollback
 *
 *
 * \brief
 * <b>Concept</b>  A register of rollback epochs organised by table name
 *
 * \brief
 * <b>Purpose</b> Simplify epoch rollback management by extracting info.
 * from DBI registry and serving by table name
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <map>
#include <string>

#ifndef ROOT_Rtypes
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#endif

namespace CP{
class TDbiRegistry;
}

namespace CP {
class TDbiEpochRollback
{

public:

  typedef std::map<std::string,UInt_t> name_map_t;

// Constructors and destructors.
           TDbiEpochRollback();
  virtual ~TDbiEpochRollback();

// State testing member functions

  const std::string& GetEpochCondition(const std::string& tableName) const;
  void Show() const;

// State changing member functions

  void Clear() {fTableToEpoch.clear();}
  void Set(TDbiRegistry& reg);  //Modifies TDbiRegistry

private:

// Data members

/// Look-up table name -> maximum epoch number
  name_map_t fTableToEpoch;

ClassDef(TDbiEpochRollback,0)   // Rollback dates by table name.

};
};


#endif  // DBIEPOCHROLLBACK_H

