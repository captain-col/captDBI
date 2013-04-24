
#ifndef DBISIMFLAGASSOCIATION
#define DBISIMFLAGASSOCIATION

/**
 *
 * $Id: TDbiSimFlagAssociation.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiSimFlagAssociation
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

#include "DbiSimFlag.hxx"

#include <iosfwd>
#include <list>
#include <map>

namespace CP {
    class TDbiSimFlagAssociation;
    class TDbiRegistry;
    std::ostream& operator<<(std::ostream& s, 
                             const CP::TDbiSimFlagAssociation& simFlagAss);

    class TDbiSimFlagAssociation {


    public:

// Typedefs for tired fingers.
        typedef std::list<CP::DbiSimFlag::SimFlag_t>           SimList_t;
        typedef std::map<CP::DbiSimFlag::SimFlag_t,SimList_t > SimMap_t;

// Constructors and destructors.
        TDbiSimFlagAssociation();
        virtual ~TDbiSimFlagAssociation();

// State testing member functions

        SimList_t Get(const CP::DbiSimFlag::SimFlag_t value)const;
        void Print(std::ostream& s)const;
        void Show();

///  Get access to the one and only instance.
        static const TDbiSimFlagAssociation& Instance();

// State changing member functions.

        void Clear() {
            fAssociations.clear();
        }
        void Set(const CP::DbiSimFlag::SimFlag_t value, SimList_t list) {
            fAssociations[value] = list;
        }
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
