// $Id: TDbiAsciiTablePreparer.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $

/////////////////////////////////////////////////////////////////////
//
// ND::TDbiAsciiTablePreparer
//
// Acknowledgments
//    The code is essentially a translation of
//    RDBC/TSQLImporterClient by Valeriy Onuchin 21/03/2001
//
/////////////////////////////////////////////////////////////////////


#include <fstream>
#include <sstream>

#include "TString.h"
#include "TSystem.h"
#include "TUrl.h"
#include "TSocket.h"

#include "TDbiAsciiTablePreparer.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;

ClassImp(ND::TDbiAsciiTablePreparer)

//   Definition of static data members
//   *********************************


//   File static non-members functions
//   *********************************

//___________________________________________________________________
TString Validate(const TString& str)
{
   // internal use static func.
   //
   // - Does validation of string as coulmn names/types, very primitive so far.
   // - Returns corrected column string

   TString ret = str.Strip(TString::kBoth);
   Int_t spidx = 0;
   const char* s = ret.Data();

   if(s[0]=='\"' || s[strlen(s)-1]=='\"' ) {
      TString quote('\"');
      ret.ReplaceAll(quote,"");
      goto exit;
   }
   if( ret.IsNull() ||
      ((ret.Length()==1) && !isalpha(s[0]) ) ) return "wrong format";

   for (Ssiz_t i = 0; i < ret.Length(); i++) {
      if( !isalnum(s[i]) && !isspace(s[i]) &&
         s[i]!=')' && s[i]!='(' && s[i]!=',' &&
         s[i]!='_' && s[i]!='-' ) {
         return "wrong format";
      }
      if(isspace(s[i])) spidx = i;
   }

exit:
   if(!spidx) ret += " TEXT NOT NULL";
   return ret;
}

//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.


//___________________________________________________________________

ND::TDbiAsciiTablePreparer::TDbiAsciiTablePreparer(const TString& url)
{
   // ctor.


   DbiTrace( "Creating ND::TDbiAsciiTablePreparer "  << (void*) this << "  ");

   fUrl = new TUrl(url);
   fStatus = 0;
   TString host(fUrl->GetHost());

   TString str(fUrl->GetFile());
   fLocalFile = str;

   fTableName = TString(gSystem->BaseName(fLocalFile.Data()));
   TString ext = strrchr(fTableName.Data(),'.');

   if(!ext.IsNull()) {
      Int_t pidx = fTableName.Index(ext.Data());
      if(pidx>1) {
         fTableName =  fTableName(0,pidx);
      }

      fTableName.ReplaceAll(".","_");
   }


   if( host=="localhost" || host.IsNull() )  {
      fMustDeleteLocalFile = kFALSE;
      DbiLog( "Preparing table " << fTableName
				 << " from local file " << fLocalFile << "  ");
   } else {
      fMustDeleteLocalFile = kTRUE;
      fLocalFile = Form("/tmp/%s%d",gSystem->BaseName(fUrl->GetFile()),gSystem->GetPid());
      DbiLog( "Preparing table " << fTableName
				 << " by downloading remote file " << fUrl->GetFile()
				 << " from remote host " << host
				 << " to local file " << fLocalFile << "  ");
      GET(url);   // download
   }

   fSkipLines = 1; // default , first line is a header describes the columns

   this->Init();
}

//___________________________________________________________________
ND::TDbiAsciiTablePreparer::~TDbiAsciiTablePreparer()
{
   // dtor.


   DbiTrace( "Destroying ND::TDbiAsciiTablePreparer "  << (void*) this << "  ");

   Clean();
}

//___________________________________________________________________
void ND::TDbiAsciiTablePreparer::Clean()
{
   //

   if(fMustDeleteLocalFile) {
      gSystem->Unlink(fLocalFile.Data());
   }
   if(fUrl) delete fUrl;
}


//___________________________________________________________________
void ND::TDbiAsciiTablePreparer::GET(const TString& url)
{
   // Download url into local temporary file

   TString str;
   const Int_t buflen=8192;
   static char buf[buflen];

   TString filename = url;
   filename.ReplaceAll(" ","");

   TUrl u(filename);

   TSocket s(u.GetHost(), u.GetPort());

   if (!s.IsValid()) {
     std::ostringstream oss;
     oss << "Unable to open socket to host " << u.GetHost()
	 <<" port " <<  u.GetPort();
     fExceptionLog.AddEntry(oss.str());
     fStatus = HTTP_FORBIDDEN;
     return;
   }

   TString msg = Form("GET %s HTTP/1.0\015\012\015\012", u.GetFile());
   s.SendRaw(msg.Data(), msg.Length());

   while(s.RecvRaw(buf, buflen)>0) {
      str += buf;
      memset(buf,0,buflen);
   }
   s.Close();

   // cutoff HTTP header
   Int_t idx;
   idx = str.Index("\015\012\015\012");
   if(idx!=kNPOS) str = str(idx+4,str.Length()-idx-4);

   std::ofstream out_file(fLocalFile.Data());
   if(!out_file) {
     std::ostringstream oss;
     oss << "Unable to open to " << fLocalFile << " for writing";
     fExceptionLog.AddEntry(oss.str());
     fStatus = HTTP_FORBIDDEN;
   }

   else {
     out_file << str;
     if( out_file.fail() )  {
        std::ostringstream oss;
        oss << "Unable to write to " << fLocalFile;
        fExceptionLog.AddEntry(oss.str());
        fStatus = HTTP_FORBIDDEN;
     }
   }
   out_file.close();
   return;
}

//___________________________________________________________________
Int_t ND::TDbiAsciiTablePreparer::Init()
{
   // - read first line from local file
   // - determine column names and types

   TString str;

   if(gSystem->AccessPathName(fLocalFile.Data())) {
      fStatus = HTTP_NOT_FOUND;
      str = "File ";
      str += fLocalFile + " not found";
      fExceptionLog.AddEntry(str.Data());
      return fStatus;
   }

   ifstream in_file(fLocalFile.Data());

   if( !in_file ) {
      in_file.close();
      fStatus = HTTP_FORBIDDEN;
      str = "You don't have read permission to ";
      str += fLocalFile;
      fExceptionLog.AddEntry(str.Data());
      return fStatus;
   }

   const Int_t buflen=8192;
   char buf[buflen];

   in_file.getline(buf,buflen);  // read first line
   str = buf;

   if(str.IsNull()) {
      in_file.close(); // empty file
      fStatus = HTTP_NOT_ACCEPTABLE;
      str = "File ";
      str += fLocalFile + " is empty";
      fExceptionLog.AddEntry(str.Data());
      return fStatus;
   }

   TString tmp;
   Int_t i,k;
   Int_t ncols = 0;
   Bool_t wrongFormat = kFALSE;

   for( i=k=0; (i=str.Index(",",i))>0; k=i++ ) {
      ncols++;
      tmp = Validate(str(!k?0:k+1,!k?i:i-k-1));
      wrongFormat = wrongFormat || tmp.IsNull() || (tmp=="wrong format");
      if(!wrongFormat) fColumns +=  tmp + ",";
   }

   ncols++;
   tmp = Validate(str(k+(ncols>1),str.Length())); // the rest of string

   wrongFormat = wrongFormat || (tmp=="wrong format");
   if(!wrongFormat)  {
      fColumns += tmp;
   }
   else {
      fColumns = "";
      for(i=1; i<ncols; i++) fColumns += Form("C%d TEXT NOT NULL,",i);
      fColumns += Form("C%d TEXT NOT NULL",ncols);
      fSkipLines = 0;
      DbiWarn( "Missing header line; treating first line as data" << "  ");
   }

   in_file.close();
   return fStatus = HTTP_OK;
}


