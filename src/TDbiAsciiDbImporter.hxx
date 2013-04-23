#ifndef DBIASCIIDBIMPORTER_H
#define DBIASCIIDBIMPORTER_H

/**
 *
 
 * \class ND::TDbiAsciiDbImporter
 *
 *
 * \brief
 * <b>Concept</b> A utility to prepare an ASCII database from a
 *  a collection of ASCII files.
 *
 * 
 * <b>Purpose</b> To prepare a temporary (process specific) ASCII database.
 *
 * 
 * <b>Acknowledgments</b> The code is essentially a translation of
 *    RDBC/TSQLImporter by Valeriy Onuchin 21/03/2001
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 *
 */

#include <list>
#include <string>

#include "TDbiExceptionLog.hxx"

namespace ND {
class TDbiAsciiTablePreparer;
}
class TSQLServer;
class TString;


namespace ND {
class TDbiAsciiDbImporter
{
public:
   TDbiAsciiDbImporter();
   TDbiAsciiDbImporter(const TString& url,TSQLServer* server);
   virtual ~TDbiAsciiDbImporter();

   const TDbiExceptionLog& GetExceptionLog() const { return fExceptionLog; }
   Int_t  Import(const TString& url,TSQLServer* server);
   const std::list<std::string>& GetImportedTableNames() const { return fImportedTableNames; }
   Bool_t IsValid() const;


private:
   void  LoadCatalog(const TString& url);
   void  LoadTable(const TString& url);


/// Status of import procedure, fStatus < 400 status is OK
   Int_t fStatus;

/// Log of exceptions generated.
   TDbiExceptionLog fExceptionLog;

/// List of imported table names
     std::list<std::string> fImportedTableNames;


/// Where to import data. Not owned.
   TSQLServer* fServer;

/// Helper class used to prepare a single table.  May be null
   TDbiAsciiTablePreparer* fTablePreparer;


ClassDef(TDbiAsciiDbImporter,0)// class used to create a temporary ASCII database
};
};


#endif // DBIASCIIDBIMPORTER_H

