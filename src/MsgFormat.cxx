////////////////////////////////////////////////////////////////////////
// $Id: MsgFormat.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $
//
// MsgFormat
//
// This implements the format classes as found in Stroustrup's book
//
// messier@huhepl.harvard.edu
////////////////////////////////////////////////////////////////////////
#include <MsgFormat.hxx>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <MsgBoundFormat.hxx>

//......................................................................

std::ostream& operator<<(std::ostream& os, const MsgBoundFormat& bf) 
{
//======================================================================
// Purpose: Print a value with the given format
//
// Inputs: os - the stream to print to
//         bf - the value and format to print
//
// Returns: the stream printed to
//======================================================================
  int p = os.precision();
  MsgFormat::fmtflags f = 
    os.setf(bf.f.fmt,static_cast<MsgFormat::fmtflags>(0xFFFF));
  char c = os.fill();

  if (bf.f.flc != c) os.fill(bf.f.flc);

  if ((bf.f.fmt&std::ios::hex) || (bf.f.fmt&std::ios::oct)) {
    os.setf(bf.f.fmt, std::ios::basefield);
    os << std::setprecision(bf.f.prc) << std::setw(bf.f.wdt) << (int)bf.val;
  }
  else {
    os << std::setprecision(bf.f.prc) << std::setw(bf.f.wdt) << bf.val;
  }

  // Reset stream
  if (c != os.fill()) os.fill(c);  
  os.precision(p); 
  os.flags(f);
  return os;
}

//......................................................................

MsgFormat::MsgFormat(const char* f) 
{
//======================================================================
// Purpose: Create a message format using given formatting flags
//
// Inputs: f - format string a la printf (see man format)
//======================================================================
  int i, j=0, k=0;
  char c, n[2][16];
  prc = 6;
  wdt = 0;
  fmt = static_cast<fmtflags>(0);
  flc = ' ';
  for (i=0; f[i] != '\0'; ++i) {
    switch(c = f[i]) {
    case '-': fmt |= std::ios::left; break;
    case '+': fmt |= std::ios::showpos; break;
    case ' ': break;
    case 'O': flc = '0'; fmt |= std::ios::left; break;
    case 'd': break;
    case 'u': break;
    case 'i': break;
    case 'o': fmt |= std::ios::oct; fmt |= std::ios::showbase; break;
    case 'x': fmt |= std::ios::hex; fmt |= std::ios::showbase; break;
    case 'X': fmt |= std::ios::hex; fmt |= std::ios::showbase; break;
    case 'f': fmt |= std::ios::fixed; break;
    case 'e': fmt |= std::ios::scientific; break;
    case 'E': fmt |= std::ios::scientific; break;
    case 'g': break;
    case 'G': break;
    default:
      if (c == '.') {
	n[j][k] = '\0';
	++j;
	k = 0;
      }
      else {
	if (c >= '0' && c <= '9') n[j][k++] = c;
      }
      break;
    }
  }
  if (k!=0 || j!=0) {
    if (j==0) n[0][k] = '\0';
    wdt = atoi(n[0]);
  }
  if (j>0) {
    n[1][k] = '\0'; 
    prc = atoi(n[1]); 
  }
}
