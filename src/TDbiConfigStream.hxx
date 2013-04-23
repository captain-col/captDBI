#ifndef DBICONFIGSTREAM_H
#define DBICONFIGSTREAM_H

/**
 *
 * $Id: TDbiConfigStream.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
 *
 * \class ND::TDbiConfigStream
 *
 *
 * \brief
 * <b>Concept</b>  I/O of configuration data and equivalent TDbiRegistry objs.
 *
 * \brief
 * <b>Purpose</b> Used to fill/store TDbiRegistry objects.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <string>

#include "TDbi.hxx"
#include "TDbiConfigSet.hxx"
#include "TDbiResultSetHandle.hxx"
#include "TDbiRegistry.hxx"
#include "TVldContext.hxx"

namespace ND {
class TDbiConfigStream;
class TDbiFieldType;
}
namespace ND{
class TDbiRegistry;
}
#include <iosfwd>

ostream& operator<<(ostream& s, const ND::TDbiConfigStream& cfStream);

namespace ND {
class TDbiConfigStream
{


public:

static      ND::TVldContext fgDefaultContext;

// Constructors and destructors.
           TDbiConfigStream();
           TDbiConfigStream(const std::string& SoftwName,
                           const std::string& ConfigName="default",
                           ND::TVldContext vc = TDbiConfigStream::fgDefaultContext,
                           TDbi::Task task=0,
                           const std::string& tableName="SOFTWARE_CONFIGURATION");
  virtual ~TDbiConfigStream();

// State testing member functions
  const ND::TDbiConfigStream& operator>>(TDbiRegistry* reg);
  const TDbiConfigSet* GetConfigSet() const { return fCFSet; }

// State changing member functions

  ND::TDbiConfigStream& operator<<(const TDbiRegistry* reg);
               Bool_t IsEmpty() { return fCFSet == 0; }
      TDbiValidityRec& GetValidityRec() { return fVRec; }
               Bool_t Write(UInt_t dbNo = 0,
                            const std::string& logComment = "",
                            Bool_t localTest = false);

private:

  TDbiConfigStream(const TDbiConfigStream&);  // Not implimented

// Data members

  ///  Pointer to the associated configuration set.  May point either
  ///  to a row of fCFSetTable (or 0) or, after operator <<, to fCFSetModified
  const TDbiConfigSet*
                 fCFSet;

  /// Pointer to ConfigSet table
  ND::TDbiResultSetHandle<TDbiConfigSet>
                 fCFSetTable;

  /// Local ND::TDbiConfigSet (empty until/unless filled using operator <<).
    TDbiConfigSet fCFSetModified;

  /// Associated validity rec (used when writing back to the database).
  TDbiValidityRec fVRec;

  /// Name of the configuration set
  std::string    fConfigName;

  /// Name of the software system to be configured
  std::string    fSoftwName;

   ClassDef(TDbiConfigStream,0)  // Input/output stream of configuration data

};
};

#endif  // DBICONFIGSTREAM_H

