////////////////////////////////////////////////////////////////////////
// $Id: UtilString.cxx,v 1.2 2012/06/14 10:55:23 finch Exp $
//
// A collection of string utilities beyond what's normally available
//
// messier@huhepl.harvard.edu
////////////////////////////////////////////////////////////////////////
#include <cstring>
#include <cstdlib>

#include "UtilString.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
using std::string;



//......................................................................

void ND::UtilString::MakePrintable(const char* in,
                               std::string& out)
{
//======================================================================
// Append string 'in' to 'out ' in a form that can be printed within
// double or single quotes.
//======================================================================

  bool hasSpecial = false;
  int index       = 0;
  while ( unsigned char c = in[index++]  ) {
    if (     c == '\\'   || c == '\n'    || c == '\t'     || c == '\'' || c == '\"'
         ||  c <= '\x08' || (c >= '\x0b' && c <= '\x1f' ) || c >= '\x7f' ) {
      hasSpecial = true;
      break;
    }
  }
  if ( ! hasSpecial ) {
    out += in;
    return;
  }
  index = 0;
  while ( unsigned char c = in[index++] ) {
    // Skip really unprintable ones.
    if ( c <= '\x08' || (c >= '\x0b' && c <= '\x1f' ) || c >= '\x7f' ) continue;
    if  ( c == '\\' || c == '\n' || c == '\t' || c == '\'' || c == '\"') {
      switch ( c ) {
	case '\\': out += "\\\\"; break;
	case '\n': out += "\\n";  break;
	case '\t': out += "\\t";  break;
	case '\'': out += "\\\'"; break;
	case '\"': out += "\\\""; break;
      }
    }
    else out += c;
  }
}

//......................................................................

void ND::UtilString::StringTok(std::vector<std::string>& ls,
			   const std::string& str,
			   const std::string& tok)
{
//======================================================================
// Split a long string into a set of shorter strings spliting along
// divisions makers by the characters listed in the token string
//======================================================================
  const string::size_type S     = str.size();
  const string::size_type toksz = tok.size();
  string::size_type  i = 0;

  while (i < S) {
    // eat leading whitespace
    while ( (i<S) && (tok.find(str[i])<=toksz) ) {
      ++i;
    }
    if (i == S)  return;  // nothing left but WS

    // find end of word
    string::size_type  j = i+1;
    while ( (j<S) && !(tok.find(str[j])<=toksz) ) {
      ++j;
    }

    // add word
    ls.push_back(str.substr(i,j-i));

    // set up for next loop
    i = j+1;
  }
}

//......................................................................

bool ND::UtilString::IsInt(const char* s)
{
//======================================================================
// Does the string s represent an integer?
//======================================================================
  char* endptr;
  double d = strtod(s, &endptr);
  if (endptr==s && d==0.0) return false; // Conversion to double failed...

  // Check if this number is int or float
  if (strchr(s,'.')) return false;
  if (strchr(s,'E')) return false;
  if (strchr(s,'e')) return false;

  // All checks for "intness" passed
  return true;
}


//......................................................................

bool ND::UtilString::IsFloat(const char* s)
{
//======================================================================
// Does the string s represent an integer?
//======================================================================
  char* endptr;
  double d = strtod(s, &endptr);
  if (endptr==s && d==0.0) return false; // Conversion to double failed...

  // All checks for "floatness" passed
  return true;
}

//......................................................................

bool ND::UtilString::IsBool(const char* s)
{
//======================================================================
// Can the string value be interpreted as a bool value?
//======================================================================
  bool isvalid;
  atob(s,isvalid);
  return isvalid;
}

//......................................................................

bool ND::UtilString::atob(const char* s)
{
//======================================================================
// Convert the text string to its bool equivalent No error checking is
// done. Returns "false" if the contents of value are not recognized
//======================================================================

  bool isvalid;
  bool value = atob(s,isvalid);
  if (isvalid) return value;

  // Oops, what have we here?
  DbiWarn(  "Attempt to convert string '" << value << "' to bool. Result is 'false'");
  return false;

}

//......................................................................

bool ND::UtilString::atob(const char* s, bool& isvalid)
{
//======================================================================
// Convert the text string to its bool equivalent No error checking is
// done. Returns "false" if the contents of value are not regognized
//======================================================================
  isvalid = true;

  std::string v(s);
  if (v == "true")   return true;  // C++ style
  if (v == "false")  return false;
  if (v == "kTRUE")  return true;  // ROOT style
  if (v == "kFALSE") return false;
  if (v == "TRUE")   return true;  // Some other reasonable variations...
  if (v == "FALSE")  return false;
  if (v == "True")   return true;
  if (v == "False")  return false;
  if (v == "on")     return true;
  if (v == "off")    return false;
  if (v == "On")     return true;
  if (v == "Off")    return false;
  if (v == "ON")     return true;
  if (v == "OFF")    return false;

  isvalid = false;
  return false;  // by default invalid strings are false
}

//......................................................................

int ND::UtilString::cmp_nocase(const std::string& s1, const std::string& s2) {
//======================================================================
// compare two strings without caring about case
// taken from Stroustrup Special Ed, 20.3.8
//======================================================================
  std::string::const_iterator p1=s1.begin();
  std::string::const_iterator p2=s2.begin();
  while (p1!=s1.end() && p2!=s2.end()) {
    if (toupper(*p1) != toupper(*p2))
      return (toupper(*p1)<toupper(*p2)) ? -1 : 1;
    ++p1; ++p2;
  }
  return (s2.size()==s1.size()) ? 0 : (s1.size()<s2.size()) ? -1:1;
}
//......................................................................

int ND::UtilString::cmp_wildcard(const std::string& s, const std::string& w) {
//======================================================================
// compare two strings where second string may end with wildcard *
//======================================================================
  std::string::size_type locStar = w.find('*');
  if ( locStar == std::string::npos ) return s.compare(w);
  return strncmp(s.c_str(),w.c_str(),locStar);
}

//......................................................................

std::string ND::UtilString::ToUpper(const std::string & str) {
//======================================================================
// Convert string to upper case.
//======================================================================

  std::string out(str);
  unsigned loc = str.size();
  while ( loc ) {
    --loc;
    out[loc] = toupper(out[loc]);
  }
  return out;
}

//......................................................................

std::string ND::UtilString::ToLower(const std::string & str) {
//======================================================================
// Convert string to upper case.
//======================================================================

  std::string out(str);
  unsigned loc = str.size();
  while ( loc ) {
    --loc;
    out[loc] = tolower(out[loc]);
  }
  return out;
}

////////////////////////////////////////////////////////////////////////

