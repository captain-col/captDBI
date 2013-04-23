#ifndef DBIASCIITABLEPREPARER_H
#define DBIASCIITABLEPREPARER_H

/**
 *
 * $Id: TDbiAsciiTablePreparer.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
 *
 * \class CP::TDbiAsciiTablePreparer
 *
 *
 * \brief
 * <b>Concept</b> A helper class to prepare an ASCII database table file
 *         for importing
 *
 * 
 * <b>Purpose</b> To simplifly the construction of a temporary (process specific)
 *  ASCII database.
 *
 * 
 * <b>Acknowledgments</b> The code is essentially a translation of
 *    RDBC/TSQLImporterClient by Valeriy Onuchin 21/03/2001
 *
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */


#include "TUrl.h"
#include "TString.h"

#include "TDbiExceptionLog.hxx"

namespace CP {
class TDbiAsciiTablePreparer
{

public:
   TDbiAsciiTablePreparer(const TString& url);
   virtual ~TDbiAsciiTablePreparer();

   const TDbiExceptionLog& GetExceptionLog() const { return fExceptionLog; }
   TString GetLocal() const { return fLocalFile.IsNull() ? 0 : "LOCAL"; }
   TString GetTableName() const { return fTableName; }
   TString GetColumns() const { return fColumns; }
   TString GetLocalFile() const { return fLocalFile; }
     Int_t GetSkipLines() const { return fSkipLines; }
     Int_t GetStatus() const { return fStatus; }
    Bool_t IsValid() const { return ((fStatus >= 200)&&(fStatus < 300)); }


private:

   virtual  Int_t Init();
   virtual  void  GET(const TString& url);
   virtual  void  Clean();


/// local file will be deleted (downloaded file)
   Bool_t fMustDeleteLocalFile;

/// Log of exceptions generated.
/// Cleared by Open Close and (implicitly) by CreatePreparedStatement, GetServer
  TDbiExceptionLog fExceptionLog;

/// url
   TUrl* fUrl;

/// local file
   TString  fLocalFile;

/// table name
   TString  fTableName;

/// column names & types
   TString  fColumns;

/// status (corresponds HTTP Status Codes)
   Int_t    fStatus;

/// number of lines to skip
   Int_t    fSkipLines;

ClassDef(TDbiAsciiTablePreparer,0)// Class used to prepare a table for a temporary ASCII database
};
};

//    from /usr/include/apache/httpd.h
#ifndef APACHE_HTTPD_H
  #define HTTP_OK                            200
  #define HTTP_BAD_REQUEST                   400
  #define HTTP_FORBIDDEN                     403
  #define HTTP_NOT_FOUND                     404
  #define HTTP_NOT_ACCEPTABLE                406
#endif //APACHE_HTTPD_H


#endif // DBIASCIITABLEPREPARER_H

