////////////////////////////////////////////////////////////////////////
// $Id: TDbiCfgConfigurable.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
//
/// \class CP::TDbiCfgConfigurable
/// \brief A base class for classes configured using registries.
///\verbatim
/// Configurable classes must implement the "Config" method which
/// in order to make any necessary changes needed to reflect the
/// change in configuration.
///
/// The "Update" method can be called frequently.  It will call
/// Config() if and only if the configuration has changed since the
/// last call to Config().  If no parameters have changed, the method
/// returns very quickly as no actions are taken.
///
///
/// To use this class, you *must* do the following in your constructor:
/// class C : public TDbiCfgConfigurable
/// {
///   C:C() { 
///     // fill the default config:
///     TDbiRegistry r;
///     r.Set("a",42);
///     r.Set("b",6.9);
///     // etc.  And prime the actual configuration:
///     this->CommitDefaultConfig(r);
/// }
///
/// This ensures that the configuration used in the future starts with
/// the correct defaults.
///
/// messier@huhepl.harvard.edu
/// 
/// 2002/08/23, modifed to inherit from TDbiRegistry and
/// not call Config() in the sub constructor. bv@bnl.gov
///
/// 2002/08/29, not IsA, but HaveA TDbiRegistry. bv@bnl.gov
/// \endverbatim
////////////////////////////////////////////////////////////////////////
#ifndef CFGCONFIGURABLE_H
#define CFGCONFIGURABLE_H
#ifndef REGISTRY_H
# include "TDbiRegistry.hxx"
#endif
namespace CP {
class TDbiCfgDialog;
}

#ifndef ROOT_Rtypes
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "Rtypes.h"
#endif
#endif

namespace CP {
class TDbiCfgConfigurable
{
  
public:

  TDbiCfgConfigurable();
  virtual ~TDbiCfgConfigurable();

  /// Subclass should implement this method:
  virtual void            Config() = 0;

  /// Access default and main configuration
  virtual const TDbiRegistry& DefaultConfig() const;
  TDbiRegistry&               GetConfig();
  const TDbiRegistry&         GetConfig() const;

  /// Call anytime.  Triggers Config() only if it has been modifed
  /// since last calling.
  int  Update();

  /// Set config via interactive dialog
  void Set(CP::TDbiCfgDialog* d=0);

  /// Set config via string
  void Set(const char* s);

protected:
  virtual void CommitDefaultConfig(const TDbiRegistry& r);

private:
  TDbiRegistry fDefConfig, fConfig;
    

  ClassDef(TDbiCfgConfigurable,1)
};
};

#endif // CFGCONFIGURABLE_H
////////////////////////////////////////////////////////////////////////
