#ifndef DBIROLLBACKDATES_H
#define DBIROLLBACKDATES_H

/**
 *
 * $Id: TDbiRollbackDates.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class CP::TDbiRollbackDates
 *
 *
 * \brief
 * <b>Concept</b>  A register of rollback dates organised by table name
 *
 * \brief
 * <b>Purpose</b> Simplify rollback date management by extracting info.
 * from DBI registry and serving by table name
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#ifndef ROOT_Rtypes
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#endif

#include <map>
#include <string>

namespace CP {
    class TDbiRegistry;

    class TDbiRollbackDates {

    public:

        typedef std::map<std::string,std::string> name_map_t;

// Constructors and destructors.
        TDbiRollbackDates();
        virtual ~TDbiRollbackDates();

// State testing member functions

        const std::string& GetDate(const std::string& tableName) const;
        const std::string& GetType(const std::string& tableName) const;
        void Show() const;

// State changing member functions

        void Clear() {
            fTableToDate.clear();
            fTableToType.clear();
        }
        void Set(TDbiRegistry& reg);  //Modifies TDbiRegistry

    private:

// Data members

/// Look-up table name -> date string
        name_map_t fTableToDate;
/// Look-up table name -> time type (either "INSERTDATE" [default] or "CREATIONDATE")
        name_map_t fTableToType;

        ClassDef(TDbiRollbackDates,0)   // Rollback dates by table name.

    };
};
#endif  // DBIROLLBACKDATES_H
