#include "TDbiCfgPromptConfigurable.hxx"
#include "TDbiCfgDialog.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>
#include <string.h>
#include <stdlib.h>
#include <typeinfo>
#include "TDbi.hxx"
#include <assert.h>

ClassImp(CP::TDbiCfgPromptConfigurable)

CP::TDbiCfgPromptConfigurable::TDbiCfgPromptConfigurable() {
    ////////////////////////////////////////////////////////////////////////////////////
    /// TDbiCfgPromptConfigurable
    ///
    /// A nice base class to use with configurable objects.
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
    ///
    ///};
}

void CP::TDbiCfgPromptConfigurable::InitializeConfig(const TDbiRegistry& initConfig) {
    fConfig.UnLockKeys();
    fConfig.UnLockValues();
    fConfig.Clear();
    fConfig=initConfig;
    fConfig.SetDirty(false); // Do this whenever ConfigModified is called.
    fConfig.LockKeys();
    fConfig.UnLockValues();

    // For GUIs:
    fDefaultConfig=initConfig;

    ConfigModified();
}

Bool_t CP::TDbiCfgPromptConfigurable::Split(const char* line, char sep, std::string& a, std::string& b) {
//======================================================================
    // Purpose: Split a character string into two pieces given a
    //          separator.  Example peas::carots::beans into
    //          a = peas
    //          b = carots::beans
    //
    //          Or, peas -> a=peas, b=""
    //
    // Inputs: line - the text to split
    //         sep  - the separation character (':' in the example above)
    //         a    - text before the separator
    //         b    - text after separator
    //
    // Return value: true if a split happened, false otherwise.
    //======================================================================
    // Clear a and b strings
    bool split = false;
    a = "";
    b = "";
    const char* c = line;
    while (*c != '\0') {
        if (*c == sep) {
            split=true;
            break;
        }
        a+=(*c);
        ++c;
    }
    while (*c == sep) {
        ++c;
    };
    b = c;
    return split;
}

bool CP::TDbiCfgPromptConfigurable::IsInt(const std::string& s, Int_t& i) {
    //======================================================================
    // Does the string s represent an integer?
    //======================================================================
    const char* ss = s.c_str();
    char* endptr;
    double d = strtod(ss, &endptr);
    if (endptr==ss && d==0.0) {
        return false;   // Conversion to double failed...
    }

    // Check if this number is int or float
    if (strchr(ss,'.')) {
        return false;
    }
    if (strchr(ss,'E')) {
        return false;
    }
    if (strchr(ss,'e')) {
        return false;
    }

    // All checks for "intness" passed
    i = atoi(ss);

    return true;
}


//......................................................................

bool CP::TDbiCfgPromptConfigurable::IsFloat(const std::string& s, Double_t& val) {
//======================================================================
// Does the string s represent an integer?
//======================================================================
    char* endptr;
    double d = strtod(s.c_str(), &endptr);
    if (endptr==s && d==0.0) {
        return false;   // Conversion to double failed...
    }

    // All checks for "floatness" passed
    val = d;
    return true;
}


void CP::TDbiCfgPromptConfigurable::Set(TDbiCfgDialog* d) {
//======================================================================
// Update the configuration parameters. Allow a TDbiCfgDialog object to be
// passed in. If none is passed in use the default, text based dialog
// object.
//======================================================================
    bool deleteDialog = false;
    if (d==0) {
        d = new TDbiCfgDialog();
        deleteDialog = true;
    }

    // Set up d with the default configuration parameters
    d->SetDefault(fDefaultConfig);
    d->SetCurrent(fConfig);

    // Do the querry
    TDbiRegistry r = d->Query();
    Set(r);

    // Clean up the dialog
    if (deleteDialog) {
        delete d;
        d = 0;
    }
}

void CP::TDbiCfgPromptConfigurable::Set(const char* key, char val) {
    TDbiRegistry r;
    r.Set(key,val);
    Set(r);
}


void CP::TDbiCfgPromptConfigurable::Set(const char* key, const char* val) {
    TDbiRegistry r;
    r.Set(key,val);
    Set(r);
}



void CP::TDbiCfgPromptConfigurable::Set(const char* key, double val) {
    TDbiRegistry r;
    r.Set(key,val);
    Set(r);
}


void CP::TDbiCfgPromptConfigurable::Set(const char* key, int val) {
    TDbiRegistry r;
    r.Set(key,val);
    Set(r);
}


void CP::TDbiCfgPromptConfigurable::Set(const char* key, const TDbiRegistry& val) {
    TDbiRegistry r;
    r.Set(key,val);
    Set(r);
}


void CP::TDbiCfgPromptConfigurable::Set(const char* setstring) {
    // Parse a whole string.

    // First, look for spaces to seperate entries.
    TDbiRegistry r;
    std::string a,b;
    std::string line = setstring;
    bool split;
    do {
        split = Split(line,' ',a,b);

        // a contains the possible code.
        std::string key, val;
        if (a.size()>0) {
            if (Split(a,'=',key,val)) {
                Int_t i;
                Double_t f;
                if (IsInt(val,i)) {
                    r.Set(key.c_str(),i);
                }
                else if (IsFloat(val,f)) {
                    r.Set(key.c_str(),f);
                }
                else {
                    r.Set(key.c_str(),val.c_str());
                };


            }
            else {
                DbiWarn("Can't parse fragment: " << a << "  ");
            }
        }

        line = b;
    }
    while (split);

    // Add the data.
    Set(r);
}
///
// Granddaddy of set() routines
///
///
void CP::TDbiCfgPromptConfigurable::Set(const TDbiRegistry& stuff, Bool_t recursive) {
    if (SafeMerge(fConfig,stuff,recursive)) {
        // Something got changed, so allow user to change stuff:
        ConfigModified();
        fConfig.SetDirty(false);
    }
}
/// Sets multiple things at once in registry.
/// If anything gets modified (to a new value) it returns true.
/// Works recursively on owned registries.
/// Keeps type-safety.. better than TDbiRegistry::Merge()
///
///   Throws EBadTDbiRegistryKeys();
Bool_t CP::TDbiCfgPromptConfigurable::SafeMerge(TDbiRegistry& modify,
                                                const TDbiRegistry& stuff,
                                                Bool_t recursive) {

    // First, see if we're locked up too tight to set anything.
    if (modify.ValuesLocked()) {
        DbiWarn("Configurable has values locked. Can't set:" << "  ");
        DbiWarn(stuff << "  ");
        return false;
    }


    bool modified = false;

    // Iterate through all the keys in stuff.
    TDbiRegistry::TDbiRegistryKey keyitr = stuff.Key();
    const char* key;
    while ((key = keyitr())) {

        // Does modify have this key?
        if (!modify.KeyExists(key)) {
            // Key doesn't exist.

            if (modify.KeysLocked()) {

                // Keys are locked, so throw away with an error.
                DbiWarn("Key " << key << " is not initialized in this Configurable. Skipping." << "  ");
                DbiInfo("Current key/values are:" <<"  ");
                modify.PrettyPrint(std::cout);
                DbiSevere("FATAL: " << "\"" << key << "\" is an illegal key! Fix your script!" << "  ");
                throw EBadTDbiRegistryKeys();
                continue;

            }
            else {
                // Key exists. Go on to save this one.
            }

        }
        else {
            // Key exists.

            // Is the key the right type?
            if (modify.GetType(key)!=stuff.GetType(key)) {
                DbiWarn("Key \"" << key << "\" is not initialized as type ("
                        << modify.GetType(key).name()
                        << ") but you've tried to set it to type ("
                        << stuff.GetType(key).name()
                        << ")" << "  ");
                continue;
            }

            if (modify.GetValueAsString(key)==stuff.GetValueAsString(key)) {
                // The values are identical. So save the work and skip it.
                continue;
            }
        }

        // Switch on type:
        int    vint;
        double vdouble;
        char   vchar;
        const char* vcharstar;
        TDbiRegistry vregistry;

        const type_info& theType(modify.GetType(key));

        if ((theType==typeid(char))&&(stuff.Get(key,vchar))) {
            modify.Set(key,vchar);
        }
        if ((theType==typeid(int))&&(stuff.Get(key,vint))) {
            modify.Set(key,vint);
        }
        if ((theType==typeid(double))&&(stuff.Get(key,vdouble))) {
            modify.Set(key,vdouble);
        }
        if ((theType==typeid(const char*))&&(stuff.Get(key,vcharstar))) {
            modify.Set(key,vcharstar);
        }
        if (stuff.GetTypeAsString(key) == "TDbiRegistry") {
            if (stuff.Get(key,vregistry)) {
                // Registries are harder. Merge using this function.
                if (recursive) {

                    TDbiRegistry old;
                    if (modify.Get(key,old)) {
                        TDbiRegistry old;
                        // Just for safety:
                        old.UnLockValues();
                        old.LockKeys();
                        if (SafeMerge(old,vregistry)) {
                            modified = true;
                        }
                        modify.Set(key,old);
                    }

                }
                else {
                    modify.Set(key,vregistry);
                }
            }
        }

        modified = true;
    };

    return modified;
}


