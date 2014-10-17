#ifndef TDBITABLEPROXYREGISTRY_H
#define TDBITABLEPROXYREGISTRY_H

/**
 *
 * $Id: TDbiDatabaseManager.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
 *
 * \class CP::TDbiDatabaseManager
 *
 *
 * \brief
 * <b>Concept</b>  Factory creating TDbiTableProxy objects.
 *
 * \brief
 * <b>Purpose</b> A TDbiDatabaseManager is a factory that creates
 *   and owns, TDbiTableProxys for all database tables.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#include <map>
#include <string>
#include "TDbiCfgConfigurable.hxx"
#include "TDbiSimFlagAssociation.hxx"
#include "TDbiRollbackDates.hxx"
#include "TDbiEpochRollback.hxx"



namespace CP {
    class TDbiCascader;
    class TDbiTableProxy;
    class TDbiTableRow;
    class TDbiValidate;
}

namespace CP {

    class TDbiDatabaseManager :public CP::TDbiCfgConfigurable {

        //Allow TDbiValidate access to dtor and fCascader
        friend class TDbiValidate;

    public:

        // State testing member functions
        void ShowStatistics() const;

        // State changing member functions
        static TDbiDatabaseManager& Instance();
        static       Bool_t IsActive() {
            return fgInstance ? kTRUE: kFALSE;
        }

        void Config();
        void ClearRollbacks();
        void ClearSimFlagAssociation();
        TDbiCascader& GetCascader() {
            return *fCascader;
        }
        TDbiTableProxy& GetTableProxy(const std::string& tableName,
                                      const TDbiTableRow* tableRow) ;
        void PurgeCaches();
        void RefreshMetaData(const std::string& tableName);
        void SetSqlCondition(const std::string& sql="");

    protected:

        // Constructors (protected because singleton).
        TDbiDatabaseManager();
        virtual ~TDbiDatabaseManager();

    private:

        // State changing member functions

        void SetConfigFromEnvironment();

    public:

        /// Helper class to handle deletion of singleton TDbiDatabaseManager
        struct Cleaner {
            // counts #includes
            static int fgCount;
            Cleaner() {
                this->Increment();
            }
            ~Cleaner() {
                if (--fgCount==0 && TDbiDatabaseManager::fgInstance!=0) {
                    delete TDbiDatabaseManager::fgInstance;
                    TDbiDatabaseManager::fgInstance = 0;
                }
            }
            void Increment() {
                ++fgCount;
            };
        };
        friend class Cleaner;

    private:

        // Disabled (not implemented) copy constructor and asignment.

        TDbiDatabaseManager(const TDbiDatabaseManager&);
        CP::TDbiDatabaseManager& operator=(const CP::TDbiDatabaseManager&);

        // State testing member functions
        void ApplySqlCondition() const;
        void ApplySqlCondition(TDbiTableProxy* proxy) const;

        /// Cascader
        TDbiCascader* fCascader;

        /// Default optional condition.
        std::string fSqlCondition;

#ifndef __CINT__  // Hide map from CINT; complains: missing Streamer() etc.
        /// TableName::RowName -> TableProxy
        std::map<std::string,TDbiTableProxy*> fTPmap;
#endif  // __CINT__

        /// Epoch Rollback  for each table.
        TDbiEpochRollback fEpochRollback;

        /// Rollback dates for each table.
        TDbiRollbackDates fRollbackDates;

        /// Lists of SimFlag associations.
        TDbiSimFlagAssociation fSimFlagAss;


        /// Holds only instance
        static TDbiDatabaseManager* fgInstance;

        ClassDef(TDbiDatabaseManager,0)   // Singleton register TDbiTableProxys.

    };
};

#ifndef __CINT__
/// Count the number of #includes for this class
static struct CP::TDbiDatabaseManager::Cleaner __dbi_cleaner;
#endif

#endif  // TDBITABLEPROXYREGISTRY_H

