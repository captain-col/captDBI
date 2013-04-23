////////////////////////////////////////////////////////////////////////////
// $Id: TVldValidate.cxx,v 1.2 2012/06/14 10:55:23 finch Exp $
//
// ND::TVldValidate
//
// ND::TVldValidate is a test harness for Validity package
//
// Author:  R. Hatcher 2001.01.03
//
////////////////////////////////////////////////////////////////////////////

#include "TVldValidate.hxx"

#include "TVldContext.hxx"
#include "TVldRange.hxx"

#include "TFile.h"
#include "TSystem.h"

#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
using std::cout;
using std::map;
#include <MsgFormat.h>

#include <iomanip>
#include <map>

ClassImp(ND::TVldValidate)

//______________________________________________________________________________
ND::TVldValidate::TVldValidate()
{
   // Default constructor



}

//______________________________________________________________________________
ND::TVldValidate::~TVldValidate()
{
   // delete all the owned sub-objects

}

//______________________________________________________________________________
Bool_t ND::TVldValidate::RunAllTests()
{
   Int_t fail  = 0;
   Int_t tests = 0;

   tests++;
   if (!TestTimeStamp()) fail++;

   tests++;
   if (!TestContext()) fail++;

   tests++;
   if (!TestRange()) fail++;

   tests++;
   if (!TestFileIO()) fail++;

   DbiInfo(  "VldValidate::RunAllTests had " << fail
      << " failures in " << tests << " tests"
      << "  " << endl);

   return (!fail);

}

//______________________________________________________________________________
Bool_t ND::TVldValidate::TestTimeStamp(void)
{
   // Test ND::TVldTimeStamp behaviour

   cout << "Test ND::TVldTimeStamp" << endl << endl;

   Int_t fail = 0;
   Int_t tests = 0;

   cout << "default ctor:    " << std::flush;
   ND::TVldTimeStamp defctor;
   defctor.Print("");

   DbiInfo(  " Test output to MSG service: " << defctor << "  " << endl);

   time_t nowtime;
   time(&nowtime);
   long int nsec = 12345;

   struct timespec nowts;
   nowts.tv_sec  = nowtime;
   nowts.tv_nsec = nsec;

   DbiInfo(  " original timespec:  {" << nowtime << "," << nsec << "} " << "  ");

   ND::TVldTimeStamp vldnow(nowts);
   // do these as separate       // reuses a common buffer space
      DbiLog( " vldnow as timespec: " << vldnow.AsString("2") << "  ");
   DbiInfo(  " vldnow default AsString: " << vldnow.AsString("") << "  ");
   DbiInfo(  " vldnow local   AsString: " << vldnow.AsString("l") << "  ");

   struct timespec thents = vldnow.GetTimeSpec();

   DbiInfo(  " recovered timespec: {" << thents.tv_sec << "," << thents.tv_nsec << "} " << "  ");

   tests++;
   if (nowts.tv_sec != thents.tv_sec || nowts.tv_nsec != thents.tv_nsec) fail++;


   // test various ctor's
   // all should map to Jan 04 2001, 01:26:03 GMT
   time_t   mytime = 978571563;
   Long_t   arbitrary = 123456; // fake nsec part

   timespec mytimespec = {mytime,arbitrary};
   ND::TVldTimeStamp vtr(mytime,arbitrary);
   ND::TVldTimeStamp vtsr(mytimespec);

   Int_t    year  = 2001;
   Int_t    month = 1;
   Int_t    day   = 4;
   Int_t    hour  = 1;
   Int_t    min   = 26;
   Int_t    sec   = 3;
   Int_t    secOffset = vtr.GetZoneOffset();

   ND::TVldTimeStamp vl1(year,month,day,hour,min,sec,arbitrary);
   ND::TVldTimeStamp vl2(year,month,day,hour,min,sec,arbitrary,kFALSE,-secOffset);
   ND::TVldTimeStamp vl3(year,month,day,hour-8,min,sec,arbitrary,kTRUE,8*60*60);
   ND::TVldTimeStamp vl4(year,month,day-1,hour+16,min,sec,arbitrary,kTRUE,8*60*60);
// this will only give correct results in PST zone
   ND::TVldTimeStamp vlpst(year,month,day,hour-8,min,sec,arbitrary,kFALSE);

   ND::TVldTimeStamp vly1(year-1900,month,day,hour,min,sec,arbitrary);
   ND::TVldTimeStamp vly2(year-2000,month,day,hour,min,sec,arbitrary);

   Int_t    date = year*10000 + month*100 + day;
   Int_t    time = hour*10000 + min*100 + sec;

   ND::TVldTimeStamp vs1(date,time,arbitrary);
   ND::TVldTimeStamp vs2(date,time,arbitrary,kFALSE,-secOffset);
// these next two aren't a smart way of dealing with local time
   ND::TVldTimeStamp vs3(date-1,time+160000,arbitrary,kTRUE,8*60*60);
   ND::TVldTimeStamp vs4(date,time-80000,arbitrary,kTRUE,8*60*60);
// the next two will only give correct results in PST zone
   ND::TVldTimeStamp vspst1(date,time-80000,arbitrary,kFALSE);
   ND::TVldTimeStamp vspst2(date-1,time+160000,arbitrary,kFALSE);
   ND::TVldTimeStamp vsy1(date-19000000,time,arbitrary);
   ND::TVldTimeStamp vsy2(date-20000000,time,arbitrary);

   DbiInfo(  " current TimeOffset is " << vtr.GetZoneOffset() << "  ");

   DbiInfo( "  ");
   DbiInfo( " std (vtr) " << vtr.AsString()  << "  ");

   TString byhand = "Thu, 04 Jan 2001 01:26:03 +0000 (GMT) +   123456 nsec";
   TString byclass = vtr.AsString();
   tests++;
   if (byhand != byclass) {
      DbiInfo( " strings don't match!" << "  ");
      fail++;
   }
   else {
      DbiInfo( " strings match as they should " << "  ");
   }

   tests++;   if (!CompareTimeStamps(" vtsr     ",vtsr,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vl1      ",vl1,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vl2      ",vl2,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vl3      ",vl3,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vl4      ",vl4,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vly1     ",vly1,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vly2     ",vly2,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vs1      ",vs1,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vs2      ",vs2,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vs3      ",vs3,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vs4      ",vs4,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vsy1     ",vsy1,vtr)) fail++;
   tests++;   if (!CompareTimeStamps(" vsy2     ",vsy2,vtr)) fail++;

   if (secOffset == 28800) {
      tests++;   if (!CompareTimeStamps(" vlpst    ",vlpst,vtr)) fail++;
      tests++;   if (!CompareTimeStamps(" vspst1   ",vspst1,vtr)) fail++;
      tests++;   if (!CompareTimeStamps(" vspst2   ",vspst2,vtr)) fail++;
   }

   DbiInfo( " next test expects to be a mismatch" << "  ");
   CompareTimeStamps(" now is unlikely to match arbitrary time ",vldnow,vtr);

   DbiInfo( " Alternative formats" << "  ");
   DbiInfo( " \"\"       " << vtr.AsString("")   << "  ");
   DbiInfo( " \"l\"      " << vtr.AsString("l")  << "  ");
   DbiInfo( " \"c\"      " << vtr.AsString("c")  << "  ");
   DbiInfo( " \"lc\"     " << vtr.AsString("lc") << "  ");
   DbiInfo( " \"2\"      " << vtr.AsString("2")  << "  ");

   tests++; if (vtr.GetDate() != date) fail++;
   tests++; if (vtr.GetTime() != time) fail++;

   MsgFormat i6("%6d");

   DbiInfo(  "  GMT    GetDate: " << vtr.GetDate()
      << "   GetTime: " << i6(vtr.GetTime()) << "  ");
   DbiInfo(  "  local  GetDate: " << vtr.GetDate(kFALSE)
      << "   GetTime: " << i6(vtr.GetTime(kFALSE)) << "  ");

   DbiInfo(  "VldValidate::TestTimeStamp had " << fail
      << " failures in " << tests << " tests"
      << "  " << endl);

   return (!fail);

}

//______________________________________________________________________________
Bool_t ND::TVldValidate::CompareTimeStamps(const char *label,
                                      ND::TVldTimeStamp& test, ND::TVldTimeStamp& std)
{
   if (test == std) {
      DbiInfo( "    exact match " << "  ");
      return kTRUE;
   }
   else {
      DbiInfo( " ** mismatch ** " << "  ");
      DbiInfo( "            " << test.AsString() << "  ");
      DbiInfo( "            " << std.AsString() << "  ");
      return kFALSE;
   }
}

//______________________________________________________________________________
Bool_t ND::TVldValidate::TestContext(void)
{
   // Test ND::TVldContext

   cout << "Test ND::TVldContext" << endl << endl;

   Int_t fail = 0;

   map<ND::TVldContext,long> vldc_map;
   long int entry = 0;

   ND::TVldContext defctor;
   DbiInfo( "VldContext default ctor: "
                         << defctor.AsString("") << "  ");
   vldc_map[defctor] = entry++;
//   gSystem->Sleep(2000); // sleep for 2 sec so timestamp advances

   ND::TVldContext defctor2;
   vldc_map[defctor2] = entry++;
//   gSystem->Sleep(2000); // sleep for 2 sec so timestamp advances

   // for the same timestamp try different combinations of detector & simflag
   // in the expected order that it will sort
   // (detector primary over simflag --> detector in outer loop)
   DbiInfo(  "VldContext test map<ND::TVldContext,long>" << "  ");
   ND::TVldTimeStamp now;
   for (Int_t ibit_det = 0; ibit_det<3; ibit_det++) {
      ND::DbiDetector::Detector_t det =
         (ND::DbiDetector::Detector_t) (1<<ibit_det);
      for (Int_t ibit_sim = 0; ibit_sim<4; ibit_sim++) {
         ND::DbiSimFlag::SimFlag_t sim = (ND::DbiSimFlag::SimFlag_t) (1<<ibit_sim);

         ND::TVldContext vldc(det,sim,now);
//         MSG("Vld",Msg::kInfo)
//            << " entry " << setw(2) << entry << " : "
//            << vldc.AsString()
//            << endl;
         vldc_map[vldc] = entry++;

      }
   }
   typedef map<ND::TVldContext,long>::const_iterator vcl_itr;
   int expect = 0;
   bool pass  = true;
   for (vcl_itr p = vldc_map.begin(); p != vldc_map.end(); ++p, ++expect) {
      ND::TVldContext ac = p->first;
      long       al = p->second;
      if (al != expect) pass = false;
      DbiInfo(  "   " << std::setw(2) << expect << " ? " << std::setw(2) << al << " : "
         << ac.AsString()
         << "  ");
   }
   if (!pass) fail++;


   DbiInfo(  " Test output to MSG service: " << defctor << "  ");

   DbiInfo(  "VldValidate::TestContext had " << fail << " failures "
      << "  " << endl);

   return (!fail);
}

//______________________________________________________________________________
Bool_t ND::TVldValidate::TestRange(void)
{
   // Test ND::TVldRange

   cout << "Test ND::TVldRange" << endl << endl;

   Int_t fail = 0;

   cout << "default ctor:          ";
   ND::TVldRange defctor;
   defctor.Print("");

   DbiInfo(  " Test output to MSG service: " << defctor << "  ");

   DbiInfo(  "VldValidate::TestRange had " << fail << " failures "
      << "  " << endl);

   return (!fail);

}

//______________________________________________________________________________
Bool_t ND::TVldValidate::TestFileIO(void)
{
   // Test write/read Validity objects to/from a file

   cout << "Test file IO" << endl << endl;

//   UInt_t sleep_msec = 2000;

   // ****************** OUTPUT *******************************

   TFile fout("vld_io.root","RECREATE");

   ND::TVldTimeStamp out_ts;
   ND::TVldContext   out_c;
   ND::TVldRange     out_r;

//VldTimeStamp not derived from TObject
//   out_ts.Write();
   out_c.Write();
   out_r.Write();

   fout.ls();
   fout.Close();

   // ****************** Pause *******************************

   DbiInfo( "  " << endl);
//   gSystem->Sleep(sleep_msec);
   ND::TVldContext pause_c;

   // ****************** INPUT *******************************

   TFile fin("vld_io.root","READ");
   fin.ls();

//   ND::TVldTimeStamp *in_ts = dynamic_cast<VldTimeStamp*>(fin.Get("VldTimeStamp"));
   ND::TVldContext *in_c = dynamic_cast<ND::TVldContext*>(fin.Get("VldContext"));

   fin.Close();

   // ****************** Comparison *******************************

   DbiInfo( "  " << endl);

//   gSystem->Sleep(sleep_msec);
   ND::TVldContext final_c;

//   out_ts.Print();
//   in_ts->Print();

   DbiInfo( " wrote  ND::TVldContext: " << out_c << "  ");
   DbiInfo( " interm ND::TVldContext: " << pause_c << "  ");
   DbiInfo( " read   ND::TVldContext: " << (*in_c) << "  ");
   DbiInfo( " final  ND::TVldContext: " << final_c << "  ");

   Int_t fail = 0;

//   cout << "default ctor:          ";
//   ND::TVldRange defctor;
//   defctor.Print("");


   DbiInfo(  "VldValidate::TestRange had " << fail << " failures "
      << "  " << endl);

   return (!fail);

}

//______________________________________________________________________________

