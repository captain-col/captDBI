
#ifndef DBISIMFLAGASSOCIATION
#define DBISIMFLAGASSOCIATION

/**
 *
 * $Id: TDbiSimFlagAssociation.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class ND::TDbiSimFlagAssociation
 *
 *
 * \brief
 * <b>Concept</b> Association of a particular SimFlag type with a list
 *  of SimFlag types.
 *
 * \brief
 * <b>Purpose</b> To allow the DBI to choose an alternative SimFlag type
 * when attempting to satisfy queries.  For example, allow MC data to
 * use Data constants.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include <iosfwd>
using std::ostream;
#include <list>
using std::list;
#include <map>
using std::map;

#include "DbiSimFlag.hxx"

namespace ND {
class TDbiSimFlagAssociation;
}
namespace ND{
class TDbiRegistry;
}

ostream& operator<<(ostream& s, const ND::TDbiSimFlagAssociation& simFlagAss);

namespace ND {
class TDbiSimFlagAssociation
{


public:

// Typedefs for tired fingers.
  typedef list<ND::DbiSimFlag::SimFlag_t>           SimList_t;
  typedef map<ND::DbiSimFlag::SimFlag_t,SimList_t > SimMap_t;

// Constructors and destructors.
           TDbiSimFlagAssociation();
  virtual ~TDbiSimFlagAssociation();

// State testing member functions

  SimList_t Get(const ND::DbiSimFlag::SimFlag_t value)const;
                      void Print(ostream& s)const;
                      void Show();

///  Get access to the one and only instance.
  static const TDbiSimFlagAssociation& Instance();

// State changing member functions.

  void Clear() { fAssociations.clear(); }
  void Set(const ND::DbiSimFlag::SimFlag_t value, SimList_t list) {
                                          fAssociations[value] = list; }
  void Set(TDbiRegistry& reg);

private:

///  The one and only instance (owned by TDbiDatabaseManager).
  static const TDbiSimFlagAssociation* fgInstance;

// Data members

 SimMap_t fAssociations;

 ClassDef(TDbiSimFlagAssociation,0)  // Association between SimFlag types.

};
};


#endif // DBISIMFLAGASSOCIATION

