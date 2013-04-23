#ifndef CFGPROMPTCONFIGURABLE
#define CFGPROMPTCONFIGURABLE

////////////////////////////////////////////////////////////////////////////////////
///
/// \class ND::TDbiCfgPromptConfigurable
///
/// \brief A nice base class to use with configurable objects.
/// 
/// Like the TDbiCfg(Lazy)Configurable class, but doesn't use
/// lazy execution.
///
/// How to use it:
/// Set up defaults in the constructor (or somehwere else that happens soon)
/// and call InitializeConfig() with them.
///
/// At any time, use the Set() commands to modify your object.
/// By default, your object has keys locked and values unlocked:
/// This means that:
///    - If you try to set a key that doesn't exist, it fails. (Typo protection)
///    - If you try to set a key to different type, it fails. (Type protection)
///
/// Note that the Set() commands include:
/// Set( key, int )           | 
/// Set( key, double )        |- Work like TDbiRegistry() commands, but with above safeties 
/// Set( key, const char* )   |
/// Set( key, TDbiRegistry )      |
/// Set( TDbiRegistry )            - Works like Merge(), but with above safeties
/// Set( TDbiRegistry, true)       - Ditto, but Merge()s sub-registries as well, with the same rules.
/// Set( string )              - Works like JobControl parser
///                              i.e. Set("myint=1 myfloat=2.0 mystring=blahDeblah");
///
/// If the configuration changes, ConfigModified() will be called, letting
/// you read the new configuration variables.  But it's smart: your function
/// only gets called if a variable has changed, not if they've stayed the same. 
/// 
/// Example:
///\verbatim
///class MyClass : public TDbiCfgPromptConfigurable 
///{
///   MyClass() {
///     TDbiRegistry r;
///     r.Set("Default1",1);
///     r.Set("Default2",2.0);
///     InitializeConfig(r);
///   };
///
///   void ConfigModified() {
///     GetConfig().Get("Default1",fDefault1);
///     .. etc ..
///   }
/// \endverbatim
///};


#include <string>
#include "TDbiRegistry.hxx"

namespace ND {
class TDbiCfgDialog;
} 

namespace ND {
class TDbiCfgPromptConfigurable
{
 public:
  TDbiCfgPromptConfigurable();
  TDbiCfgPromptConfigurable(const TDbiRegistry& r) {InitializeConfig(r);};
  virtual ~TDbiCfgPromptConfigurable() {};

  // For reading out current configuration.
  const TDbiRegistry& GetConfig() const { return fConfig; };

  // Wrappers for TDbiRegistry setters.
  void UnLockKeys()   { fConfig.UnLockKeys(); };
  void LockKeys()     { fConfig.LockKeys(); };
  void UnLockValues() { fConfig.UnLockValues(); };
  void LockValues()   { fConfig.LockValues(); };

  // These functions all modify the current registry, and call ConfigModified() if 
  // a valid key changes.
  void Set(const char* key, char val);
  void Set(const char* key, const char* val);
  void Set(const char* key, double val);
  void Set(const char* key, int val);
  void Set(const char* key, const TDbiRegistry& val);
  void Set(const char* setstring);  // like modules.
  void Set(ND::TDbiCfgDialog* d=0);
  

  // This is the one that does all the work:
  void Set(const TDbiRegistry& stuff, Bool_t recursive = false); // Sets multiple things at once.

  // String operation routines.
  static Bool_t Split(const char* line, char sep, std::string& a, std::string& b);
  static Bool_t Split(const std::string& line, char sep, std::string& a, std::string& b) 
  { return Split(line.c_str(),sep,a,b); };
  static Bool_t IsInt(const std::string& s, Int_t& val);
  static Bool_t IsFloat(const std::string& s, Double_t& val);

  // TDbiRegistry operation routines.
  static Bool_t SafeMerge(TDbiRegistry& modify, 
			  const TDbiRegistry& stuff, 
			  Bool_t recursive = false );


 protected:
  // This call sets up the object in it's 'default' condition,
  // and defines all the valid keys. Should be called from constructor.
  void         InitializeConfig(const TDbiRegistry& initConfig);  

  // This method is called whenever the current configuration changes in any significant way.
  virtual void ConfigModified(void)=0; // To be defined by user.

 private:
  TDbiRegistry fConfig;
  TDbiRegistry fDefaultConfig;

  ClassDef(TDbiCfgPromptConfigurable,1);
};
};

#endif
