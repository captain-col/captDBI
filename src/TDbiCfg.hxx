
/////////////////////////////////////////////////
// $Id: TDbiCfg.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
///
/// \class ND::TDbiCfg
///
/// \brief Some utility functions for working with configurations
///
/// messier@huhepl.harvard.edu
///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CFG_H
#define CFG_H
#ifndef STRING
# include <string>
# define STRING
#endif
#ifndef REGISTRY_H
# include "TDbiRegistry.hxx"
#endif

namespace ND {
class TDbiCfg
{
public:
  static void TDbiRegistryToString(std::string& s, const TDbiRegistry& r);
  static void StringToTDbiRegistry(TDbiRegistry& r,    const char* s);
private:
  TDbiCfg();
};

};
#endif // CFG_H
////////////////////////////////////////////////////////////////////////

