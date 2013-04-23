// $Id: TDbiAsciiDbImporter.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $

////////////////////////////////////////////////////////////////////
//
// CP::TDbiAsciiDbImporter
// Acknowledgments
//   The code is essentially a translation of
//   RDBC/TSQLImporter by Valeriy Onuchin 21/03/2001
//
////////////////////////////////////////////////////////////////////


#include <TSQLServer.h>
#include <TSQLStatement.h>
#include "TSystem.h"
#include <TUrl.h>

#include <TDbiAsciiDbImporter.hxx>
#include <TDbiAsciiTablePreparer.hxx>
#include <TDbiExceptionLog.hxx>
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

ClassImpQ(CP::TDbiAsciiDbImporter)

//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.


//___________________________________________________________________
CP::TDbiAsciiDbImporter::TDbiAsciiDbImporter():
  fServer(0),
  fTablePreparer(0)
{
   // ctor


     DbiTrace( "Creating CP::TDbiAsciiDbImporter " << (void*) this << "  ");

   fStatus = HTTP_BAD_REQUEST; // not OK
}

//___________________________________________________________________
CP::TDbiAsciiDbImporter::TDbiAsciiDbImporter(const TString& url,TSQLServer* server):
  fServer(server),
  fTablePreparer(0)
{
   // ctor


   DbiTrace( "Creating CP::TDbiAsciiDbImporter "  << (void*) this << "  ");

   Import(url,server);
}

//___________________________________________________________________
CP::TDbiAsciiDbImporter::~TDbiAsciiDbImporter()
{
   // dtor


   DbiTrace( "Destroying CP::TDbiAsciiDbImporter" << "  ");
   delete fTablePreparer;
   fTablePreparer = 0;
}

//___________________________________________________________________
void CP::TDbiAsciiDbImporter::LoadTable(const TString& url)
{
   //

   fStatus = HTTP_BAD_REQUEST;

   // Prepare the table for importing.
   delete fTablePreparer;
   fTablePreparer = 0;
   fTablePreparer = new CP::TDbiAsciiTablePreparer(url);

   // Check for exceptions from table preparer and include them.
   if(!fTablePreparer->IsValid()) {
      fStatus = fTablePreparer->GetStatus();
      const CP::TDbiExceptionLog& el = fTablePreparer->GetExceptionLog();
      if(! el.IsEmpty() ) fExceptionLog.AddLog(el);
      delete fTablePreparer;
      fTablePreparer = 0;
      return;
   }

   TString cols  = fTablePreparer->GetColumns();   // read column names,types
   TString table = fTablePreparer->GetTableName();
   TString file  = fTablePreparer->GetLocalFile();

   TString query("CREATE TEMPORARY TABLE ");
   query += table + "(" + cols + ")";

   DbiLog( "Creating table with: " << query << "  ");
   if(! fServer->Exec(query.Data()) ) {
      fExceptionLog.AddEntry(*fServer);
      delete fTablePreparer;
      fTablePreparer = 0;
      fStatus = HTTP_NOT_ACCEPTABLE;
      return;
   }

   query =  "LOAD DATA ";
   query += fTablePreparer->GetLocal() + " INFILE '";
   query += file;
   query += "' INTO TABLE ";
   query += table;
   query += " FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '";
   query += '\"';
   query += "'";
//   query += " ESCAPED BY '\\'";
//   query += " LINES TERMINATED BY '\n'";

   if(fTablePreparer->GetSkipLines()) {
      query += " IGNORE ";
      query += Form("%d LINES",fTablePreparer->GetSkipLines());
   }

   DbiLog( "Filling table with: " << query << "  ");
   if (! fServer->Exec(query.Data()) ) {
      fExceptionLog.AddEntry(*fServer);
      delete fTablePreparer;
      fTablePreparer = 0;
      fStatus = HTTP_NOT_ACCEPTABLE;
      return;
   }

   fImportedTableNames.push_back(table.Data());
   fStatus = HTTP_OK;
   return;
}

//___________________________________________________________________
void CP::TDbiAsciiDbImporter::LoadCatalog(const TString& url)
{

  // Laod the catalogue
   LoadTable(url);

   if( (fStatus!=HTTP_OK) || !fTablePreparer ) {
      return;
   }

   TString table = fTablePreparer->GetTableName();
   TString query = "SELECT * FROM " + table;

   DbiLog( "Reading catalogue with: " << query << "  ");

   TSQLStatement* stmt = fServer->Statement(query.Data());

   if ( ! stmt ) {
      fExceptionLog.AddEntry(*fServer);
      fStatus = HTTP_NOT_ACCEPTABLE;
      return;
   }
   stmt->EnableErrorOutput(false);
   if ( ! stmt->Process() || ! stmt->StoreResult() ) {
      fExceptionLog.AddEntry(*fServer);
      fStatus = HTTP_NOT_ACCEPTABLE;
      return;
   }

   if(fTablePreparer) {
      delete fTablePreparer;
      fTablePreparer = 0;
   }

   while(stmt->NextResultRow()) {
      table = stmt->GetString(0); // first column is URL/file
      gSystem->ExpandPathName(table);
      LoadTable(table);
   }

   if(fTablePreparer) {
      delete fTablePreparer;
      fTablePreparer = 0;
   }

   fStatus = HTTP_OK;
   if (stmt) delete stmt;
   return;
}

//___________________________________________________________________
Int_t CP::TDbiAsciiDbImporter::Import(const TString& url,TSQLServer* server)
{
   // import data from url to server

   fStatus = HTTP_BAD_REQUEST;

   if( !server ) {
      fServer = 0;
      fExceptionLog.AddEntry("No server supplied");
      return fStatus = HTTP_FORBIDDEN;
   }

   fServer = server;
   TString ext = strrchr(url.Data(),'.');  // get file extention

   DbiLog( "Importing ASCII data for " << url << "  ");

   if( (ext==".cat") || (ext==".db") ) LoadCatalog(url);
   else LoadTable(url);

   return fStatus = HTTP_OK;
}

//___________________________________________________________________
Bool_t CP::TDbiAsciiDbImporter::IsValid() const
{
   //

   return (fStatus < HTTP_BAD_REQUEST);
}



