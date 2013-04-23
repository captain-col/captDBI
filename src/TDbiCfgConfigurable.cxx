////////////////////////////////////////////////////////////////////////
// 
//
// A base class for classes configured using registries
//
// messier@huhepl.harvard.edu
////////////////////////////////////////////////////////////////////////
#include "TDbiCfgConfigurable.hxx"
#include "TDbiCfg.hxx"
#include "TDbiCfgDialog.hxx"

ClassImp(CP::TDbiCfgConfigurable)

//......................................................................

CP::TDbiCfgConfigurable::TDbiCfgConfigurable() : fConfig(false) { }

//......................................................................

CP::TDbiCfgConfigurable::~TDbiCfgConfigurable() { }

//......................................................................
///
/// Subclass must call this before the Configurable can have
/// meaningful entries
///
void CP::TDbiCfgConfigurable::CommitDefaultConfig(const TDbiRegistry& r)
{

  fDefConfig = r;
}

//......................................................................
//
/// Eventually this might go in the database and load the
/// configuration. This would take a name or something. 
//====================================================================
const CP::TDbiRegistry& CP::TDbiCfgConfigurable::DefaultConfig() const 
{
  return fDefConfig;
}
//======================================================================
/// Returns the configuration TDbiRegistry, this is non-const as the user
/// is user is free to modify
//======================================================================
CP::TDbiRegistry& CP::TDbiCfgConfigurable::GetConfig()
{

  return fConfig;
}
//======================================================================
/// Returns the configuration TDbiRegistry.  This const version denies
/// the user any freedom to modify it, but does mean that a
/// configurable object can use it in a const method.
//======================================================================
const CP::TDbiRegistry& CP::TDbiCfgConfigurable::GetConfig() const
{

  return fConfig;
}

//......................................................................
//======================================================================
/// Update the class's state given the current configuration. If there
/// is nothing to do just return w/o taking any action. Return's 0 if
/// no action was taken, >0 if the object was reconfigured.
//======================================================================
int CP::TDbiCfgConfigurable::Update() 
{

  if (! fConfig.IsDirty()) return 0;  // Nothing to do if config is current
  this->Config();               // Send the "reconfig" message
  fConfig.SetDirty(false);      // Mark the config. as current
  return 1;
}

//......................................................................
//======================================================================
/// Update the configuration parameters. Allow a TDbiCfgDialog object to be
/// passed in. If none is passed in use the default, text based dialog
// object.
//======================================================================
void CP::TDbiCfgConfigurable::Set(TDbiCfgDialog* d) 
{

  bool deleteDialog = false;
  if (d==0) {
    d = new TDbiCfgDialog();
    deleteDialog = true;
  }

  // Set up d with the default configuration parameters
  d->SetDefault(this->DefaultConfig());
  d->SetCurrent(this->GetConfig());
  
  // Do the querry
  TDbiRegistry r = d->Query();
  this->GetConfig().UnLockValues();
  this->GetConfig().Merge(r);
  this->GetConfig().LockValues();

  // Clean up the dialog
  if (deleteDialog) { delete d; d = 0; }
}

//......................................................................
//======================================================================
/// Update the configuration given a text string s. Format:
/// "key1=true, key2=10, key3=11.1, key4='A string'"
//======================================================================
void CP::TDbiCfgConfigurable::Set(const char* s) 
{

  TDbiRegistry r;
  CP::TDbiCfg::StringToTDbiRegistry(r,s);
  this->GetConfig().UnLockValues();
  this->GetConfig().Merge(r);
  this->GetConfig().LockValues();
}

////////////////////////////////////////////////////////////////////////
