////////////////////////////////////////////////////////////////////////
// $Id: TDbiCfgDialog.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
///
/// \class CP::TDbiCfgDialog
///
/// \brief A simple class to query a user to supply values for a registry. Internally 
/// stores three CP::TDbiRegistry objects,  current, default and reset
///
/// messier@huhepl.harvard.edu
////////////////////////////////////////////////////////////////////////
#ifndef CFGDIALOG_H
#define CFGDIALOG_H
#ifndef REGISTRY_H
#include "TDbiRegistry.hxx"
#endif

namespace CP {
class TDbiCfgDialog
{
public:
  TDbiCfgDialog();
  TDbiCfgDialog(const TDbiRegistry& current, const TDbiRegistry& deflt);
  virtual ~TDbiCfgDialog();

  virtual TDbiRegistry& Query();
  
  void SetCurrent(const TDbiRegistry& r);
  void SetDefault(const TDbiRegistry& r);

protected:
  TDbiRegistry fCurrent; // Current values for configuration
  TDbiRegistry fDefault; // Default settings for configuration
  TDbiRegistry fResult;  // User inputs
};

};

#endif
////////////////////////////////////////////////////////////////////////

