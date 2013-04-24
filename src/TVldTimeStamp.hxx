/**
 * \class CP::TVldTimeStamp
 *
 * \ingroup Validity
 *
 * \brief Encapsulate the seconds and ns since EPOCH
 *
 * This extends (and isolates) struct timespec
 *
 *    struct timespec
 *       {
 *          time_t   tv_sec;   // seconds
 *          long     tv_nsec;  // nanoseconds
 *       }
 *    time_t seconds is relative to Jan 1, 1970 00:00:00 UTC
 *
 * Due to ROOT/CINT limitations TVldTimeStamp does not explicitly
 * hold a timespec struct; attempting to do so means the Streamer
 * must be hand written.  Instead we have chosen to simply contain
 * similar fields within the private area of this class.
 *
 * \note
 *       the use of time_t (and its default implementation as a 32 int)
 *       implies overflow conditions occurs somewhere around
 *       Jan 18, 19:14:07, 2038.
 *       If this experiment is still going when it becomes significant
 *       someone will have to deal with it.
 *

 *
 * \author (last to touch it) $Author: finch $
 *
 * \version $Revision: 1.2 $
 *
 * \date $Date: 2011/06/09 14:44:29 $
 *
 * Contact: R. Hatcher
 *
 * Created on: Wed Apr 13 17:53:23 2005
 *
 * Author:  R. Hatcher 2000.04.19
 *          R. Hatcher 2000.12.20 -- convert from TDatime to struct timespec
 *
 * $Id: TVldTimeStamp.hxx,v 1.2 2011/06/09 14:44:29 finch Exp $
 *
 */


#ifndef VLDTIMESTAMP_H
#define VLDTIMESTAMP_H

// TTimeStamp is the ROOT adopted version of this class
// so it has all the necessary includes
#include "TTimeStamp.h"

// make the TVldTimeStamp cout'able
#include <iosfwd>

namespace CP {
class TVldTimeStamp;
    std::ostream& operator<<(std::ostream& os, const CP::TVldTimeStamp& vldts);

class TVldTimeStamp {

   friend Bool_t operator==(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs);
   friend Bool_t operator!=(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs);
   friend Bool_t operator< (const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs);
   friend Bool_t operator<=(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs);
   friend Bool_t operator> (const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs);
   friend Bool_t operator>=(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs);

   friend CP::TVldTimeStamp operator- (const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs);

 public:

   /// Static method to return the "beginning of time" (start of Unix EPOCH)
   static TVldTimeStamp GetBOT();

   /// Static method to return the "end of time" which is sometime in
   /// the future and near or at the limit of TVldTimeStamp's ability
   /// to hold large times.
   static TVldTimeStamp GetEOT();

   /// Static method to return the "negative beginning of time", i.e. the
   /// earliest time prior to start of Unix EPOCH which is negative seconds.
   static TVldTimeStamp GetNBOT();

   /// Empty ctor (builds current time with nsec field filled as best possible)
   TVldTimeStamp();

   /// Copy
   TVldTimeStamp(const TVldTimeStamp &source)
     : fSec(source.fSec), 
     fNanoSec(source.fNanoSec) { }

   /// Assignment
   TVldTimeStamp& operator=(const TVldTimeStamp &source)
      { if (this != &source) {fSec = source.fSec; fNanoSec = source.fNanoSec;}
      return *this; }

   /// Construction from timespec struct
   TVldTimeStamp(const timespec_t &ts)
     : fSec(ts.tv_sec), fNanoSec(ts.tv_nsec)
      { NormalizeNanoSec(); }

   /// Construction from time_t and separate nsec
   TVldTimeStamp(const time_t &t, const Int_t nsec)
     : fSec(t), fNanoSec(nsec)
      { NormalizeNanoSec(); }

   /// Create a TVldTimeStamp and set it to the specified year, month,
   /// day, time, hour, minute, second and nanosec.
   /// If !isUTC then it is assumed to be the standard local time zone.
   ///
   /// If local time is PST then one can use
   ///
   ///    TVldTimeStamp(year,month,day,hour,min,sec,nsec,kFALSE,0);
   ///
   /// or
   ///
   ///    Int_t secOffset = 8*60*60;
   ///    TVldTimeStamp(year,month,day,hour,min,sec,nsec,kTRUE,8*60*60);
   TVldTimeStamp(UInt_t year, UInt_t month,
                UInt_t day,  UInt_t hour,
                UInt_t min,  UInt_t sec,
                UInt_t nsec=0, 
                Bool_t isUTC=kTRUE, Int_t secOffset=0);

   /// Create a TVldTimeStamp and set it to the specified date, time, nanosec.
   /// If !isUTC then it is assumed to be the standard local time zone.
   TVldTimeStamp(UInt_t date, UInt_t time, UInt_t nsec, 
                Bool_t isUTC=kTRUE, Int_t secOffset=0);

   /** Create a TVldTimeStamp using double precision floating point
    * seconds from the EPOCH.
    *
    * \warning This will truncate precision to no better than about 1
    * microsecond.  Do not use this constructor for timestamps that
    * are expected to be more precise!
    */
   TVldTimeStamp(Double_t seconds)
     : fSec((Int_t)seconds), fNanoSec((Int_t)((seconds-fSec)*1.0e9))
     { NormalizeNanoSec(); }


   virtual ~TVldTimeStamp();

   /** Implicitly convert a TVldTimeStamp to a double.
    *
    * \warning This will truncate precision to no better than about 1
    * microsecond.  Do not compare/subtract TVldTimeStamps that have
    * been converted to doubles if you require the full nanosecond
    * precision!
    */
   operator double() const { return fSec + 1.0e-9 * fNanoSec; }


   /// Get timestamp as a timespec_t
   timespec_t     GetTimeSpec() const
      { timespec_t value = {fSec,fNanoSec}; return value; }

   /// Get (integral) seconds after the EPOCH
   time_t         GetSec(void) const { return fSec;}
   /// Get nanoseconds after the second
   Int_t          GetNanoSec(void) const { return fNanoSec; }
   
   /// Get time from the epoch in seconds
   Double_t GetSeconds(void) const { return fSec+(fNanoSec/1.0e9); }
    
  /** Return the date & time as a string.
   *
   * Result is pointer to a statically allocated string.
   * User should copy this into their own buffer before calling
   * this method again.  This is somewhat mitigated
   * by use of a circular buffer of strings.
   *
   * Option "l" returns it in local zone format
   * (can be applied to default or compact format).
   *
   * Default format is RFC822 compliant:
   *   "Mon, 02 Jan 2001 18:11:12 +0000 (GMT) +999999999 nsec"
   *   "Mon, 02 Jan 2001 10:11:12 -0800 (PST) +999999999 nsec"
   *
   * Option "c" compact is (almost) ISO 8601 compliant:
   *   "2001-01-02 18:11:12.9999999999Z"
   *   "2001-01-02 10:11:12.9999999999-0800"  if PST
   *      * uses "-" as date separator as specified in ISO 8601
   *      * uses "." rather than preferred "," for decimal separator
   *      * -HHMM is the difference between local and UTC (if behind, + if ahead).
   *   The "-HHMM" is replaced with "Z" if given as UTC.
   *   To be strictly conforming it should use "T" instead of the
   *   blank separating the date and time.
   *
   * Option "2" returns as {sec,nsec} integers.
   *
   * Option "s" returns "2001-01-02 18:11:12" with an implied UTC,
   * overrides "l" option.
   *
   * Internally uses a circular list of buffers to avoid problems
   * using AsString multiple times in a single statement.
   */
   const char    *AsString(Option_t *option="") const;
   void           Copy(TVldTimeStamp &vldts) const;

   /// Return date in form of 19971224 (i.e. 24/12/1997),
   /// if non-zero pointers supplied for year, month, day fill those as well
   Int_t          GetDate(Bool_t inUTC=kTRUE, Int_t secOffset=0,
                          UInt_t *year=0, UInt_t *month=0,
                          UInt_t *day=0) const;

   /// Return time in form of 123623 (i.e. 12:36:23),
   /// if non-zero pointers supplied for hour, min, sec fill those as well
   Int_t          GetTime(Bool_t inUTC=kTRUE, Int_t secOffset=0,
                          UInt_t* hour=0, UInt_t* min=0, 
                          UInt_t* sec=0) const;

   void           Add(const TVldTimeStamp& offset);
   void           Add(Double_t seconds);

   void           Print(Option_t *option="") const;

   // Utility functions

   /// Static method returning local (current) time zone offset from UTC.
   /// This is the difference in seconds between UTC and local standard time.
   static Int_t   GetZoneOffset();
  /**
   * Equivalent of standard routine "mktime" but
   * using the assumption that tm struct is filled with UTC, not local, time.
   *
   * This version *ISN'T* configured to handle every possible
   * weirdness of out-of-range values in the case of normalizing
   * the tm struct.
   *
   * This version *DOESN'T* correctly handle values that can't be
   * fit into a time_t (i.e. beyond year 2038-01-18 19:14:07, or
   * before the start of Epoch). */
   static time_t  MktimeFromUTC(tm_t* tmstruct);
   /// Is the given year a leap year.
   static Bool_t  IsLeapYear(Int_t year);
   /// Print out the "tm" structure:
   static void    DumpTMStruct(const tm_t& tmstruct);

 private:

   void           Set();
   void           Set(Int_t year, Int_t month, Int_t day,
                      Int_t hour, Int_t min, Int_t sec, 
                      Int_t nsec, Bool_t isUTC, Int_t secOffset);
   void           Set(Int_t date, Int_t time, Int_t nsec,
                      Bool_t isUTC, Int_t secOffset);
   void           NormalizeNanoSec();

   // Data members:
   // similar fields to struct timespec
   // use ROOT versions to "know" that they are platform consistent
   // 32-bit integers to avoid IO confusion.
   Int_t  fSec;
   Int_t  fNanoSec;

   ClassDef(CP::TVldTimeStamp,2)
};
}; 

#ifndef __CINT__
//=============================================================================
// Implementation details -- inlines need to be hidden from CINT
//=============================================================================

namespace CP
{
inline Bool_t operator==(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs)
   { return lhs.fSec  == rhs.fSec && 
            lhs.fNanoSec == rhs.fNanoSec; }

inline Bool_t operator!=(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs)
   { return lhs.fSec  != rhs.fSec ||
            lhs.fNanoSec != rhs.fNanoSec; }

inline Bool_t operator<(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs)
   { return lhs.fSec  < rhs.fSec ||
             ( lhs.fSec  == rhs.fSec &&
               lhs.fNanoSec <  rhs.fNanoSec   ); }

inline Bool_t operator<=(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs)
   { return lhs.fSec  < rhs.fSec ||
             ( lhs.fSec  == rhs.fSec &&
               lhs.fNanoSec <= rhs.fNanoSec   ); }

inline Bool_t operator>(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs)
   { return lhs.fSec  > rhs.fSec ||
             ( lhs.fSec  == rhs.fSec &&
               lhs.fNanoSec >  rhs.fNanoSec   ); }

inline Bool_t operator>=(const CP::TVldTimeStamp &lhs, const CP::TVldTimeStamp &rhs)
   { return lhs.fSec  > rhs.fSec ||
             ( lhs.fSec  == rhs.fSec &&
               lhs.fNanoSec >= rhs.fNanoSec   ); }

inline CP::TVldTimeStamp operator-(const CP::TVldTimeStamp& lhs, const CP::TVldTimeStamp& rhs)
{
    return TVldTimeStamp(lhs.GetSec()     - rhs.GetSec(),
			lhs.GetNanoSec() - rhs.GetNanoSec());
}
}

#endif /* __CINT__ */
#endif // VLDTIMESTAMP_H
