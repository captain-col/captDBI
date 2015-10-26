#include <map>
#include <iomanip>

#include "TBuffer.h"
#include "TObject.h"

#include "TDbiRegistry.hxx"
#include "TDbiRegistryItem.hxx"

#include <UtilStream.hxx>

#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

#include <typeinfo>
#include <iostream>
#include <sstream>
#include <cassert>

ClassImp(CP::TDbiRegistry)



//......................................................................

CP::TDbiRegistry::TDbiRegistry(bool readonly /* = true */)
    : fValuesLocked(readonly),
      fKeysLocked(false),
      fErrorHandler(0) {
    DbiTrace("Creating TDbiRegistry at " << (void*) this << "  ");
    this->SetDirty();
}

// Deep copy constructor
CP::TDbiRegistry::TDbiRegistry(const TDbiRegistry& rhs) : TNamed(rhs) {
    DbiTrace("Creating TDbiRegistry at " << (void*) this << "  ");
    TDbiRegistryKey rk = rhs.Key();
    const char* s;

    while ((s = rk())) {
        fMap[s] = rhs.fMap.find(s)->second->Dup();
    }

    fValuesLocked = rhs.fValuesLocked;
    fKeysLocked   = rhs.fKeysLocked;
    this->SetDirty();
    this->SetName(rhs.GetName());
}

CP::TDbiRegistry& CP::TDbiRegistry::operator=(const TDbiRegistry& rhs) {
    if (this == &rhs) {
        return *this;
    }

    UnLockValues();
    UnLockKeys();

    // If we are already holding something - clear it.
    if (Size() != 0) {
        Clear();
    }

    TDbiRegistryKey rk = rhs.Key();
    const char* s;

    while ((s = rk())) {
        fMap[s] = rhs.fMap.find(s)->second->Dup();
    }

    fValuesLocked = rhs.fValuesLocked;
    fKeysLocked   = rhs.fKeysLocked;
    this->SetDirty();
    this->SetName(rhs.GetName());

    // Do like copy ctor.
    return *this;
}

void CP::TDbiRegistry::Merge(const TDbiRegistry& rhs) {
    if (this == &rhs) {
        return;
    }

    TDbiRegistryKey rk = rhs.Key();
    const char* s;
    while ((s = rk())) {
        tRegMap::iterator mit = fMap.find(s);
        bool exists = mit != fMap.end();

        if (fKeysLocked && !exists) {
            DbiWarn("Merge: can't, add new key " << s <<", keys locked."
                    << "  merger=" << this->GetName()
                    << ", mergie=" << rhs.GetName() << "  ");
            continue;
        }
        if (exists && fValuesLocked) {
            DbiWarn("Merge: can't, merge key " << s <<", values locked."
                    << "  merger=" << this->GetName()
                    << ", mergie=" << rhs.GetName() << "  ");
            continue;
        }
        if (exists) {
            delete mit->second;
        }
        fMap[s] = rhs.fMap.find(s)->second->Dup();
    }
    this->SetDirty();
}

bool CP::TDbiRegistry::KeyExists(const char* key) const {
    return fMap.find(key) != fMap.end();
}

void CP::TDbiRegistry::RemoveKey(const char* key) {
    tRegMap::iterator dead = fMap.find(key);
    if (dead == fMap.end()) {
        return;
    }
    fMap.erase(dead);
    delete dead->second;
    this->SetDirty();
}

void CP::TDbiRegistry::Clear(Option_t* /* option */) {
    if (fValuesLocked || fKeysLocked) {
        DbiWarn("Clear: can't, there are locks in \""
                << this->GetName() << "\"\n");
        return;
    }

    tRegMap::iterator mit = fMap.begin();
    while (mit != fMap.end()) {
        delete mit->second;
        ++mit;
    }
    fMap.clear();
    this->SetDirty();
}

void CP::TDbiRegistry::Dump(void) const {
    this->TNamed::Dump();
    tRegMap::const_iterator mit = fMap.begin();
    DbiInfo("TDbiRegistry: `" << this->GetName() << "', "
            << this->Size() << " entries."
            << " (Locks: [Keys|Values] `key', `value'):\n");
    while (mit != fMap.end()) {
        DbiInfo(" [" << (fKeysLocked ? 'L' : 'U') << "|"
                << (fValuesLocked ? 'L' : 'U') << "] "
                << "`" << mit->first << "', `");
        mit->second->Dump();
        DbiInfo("'\n");
        ++mit;
    }

}

std::ostream& CP::TDbiRegistry::PrettyPrint(std::ostream& os) const {
    static int print_depth = 0;

    // print (to cout) the registry
    tRegMap::const_iterator mit = this->fMap.begin();
    for (int i=0; i<print_depth; ++i) {
        os << " ";
    }
    os << "\"" << this->GetName() << "\", "
       << this->Size() << " entries."
       << " keys " << (this->fKeysLocked ? "locked" : "unlocked")
       << ", values " << (this->fValuesLocked ? "locked" : "unlocked")
       << "\n";

    print_depth+=4;
    while (mit != this->fMap.end()) {
        for (int i=0; i<print_depth; ++i) {
            os << " ";
        }

        os << mit->first << " = ";
        mit->second->PrintStream(os);
        os << std::endl;
        ++mit;
    }
    print_depth-=4;
    return os;
}

void CP::TDbiRegistry::Print(Option_t* /* option */) const {
    this->PrettyPrint(std::cout);
}


CP::TDbiRegistry::~TDbiRegistry() {
    tRegMap::iterator mit = fMap.begin();
    while (mit != fMap.end()) {
        delete mit->second;
        ++mit;
    }
}

CP::TDbiRegistry::TDbiRegistryKey::TDbiRegistryKey(const TDbiRegistry* r) :
    fReg(r) {
    // FIXME!  Figure out how to correctly declare fIt to reflect
    // constness.
    fIt = const_cast<CP::TDbiRegistry*>(fReg)->fMap.begin();
}

CP::TDbiRegistry::TDbiRegistryKey::TDbiRegistryKey() {
}

CP::TDbiRegistry::TDbiRegistryKey::~TDbiRegistryKey() {
}

const char* CP::TDbiRegistry::TDbiRegistryKey::operator()(void) {
    if (fIt == fReg->fMap.end()) {
        return 0;
    }
    const char* s = fIt->first.c_str();
    ++ fIt;
    return s;
}

CP::TDbiRegistry::TDbiRegistryKey CP::TDbiRegistry::Key(void) const {
    return CP::TDbiRegistry::TDbiRegistryKey(this);
}

#define REGISTRY_SET(TYPE)                                              \
    bool CP::TDbiRegistry::Set(const char* key, TYPE val)                           \
    {                                                                       \
        tRegMap::iterator mit = fMap.find(key);                             \
        if (mit != fMap.end()) {                                            \
            if (fValuesLocked) {                                            \
                DbiWarn(  "Set: Values are locked - not overwriting \""         \
                          << key << "\" with \"" << val << "\" in registry \"" << this->GetName() << "\"\n");\
                return false;                                               \
            }                                                               \
            if (!dynamic_cast<CP::TDbiRegistryItemXxx<TYPE>*>(mit->second)) {       \
                DbiWarn(  "Set: attempt to overwrite old value for key \""     \
                          << key << "\" with different type value "               \
                          << val << " in registry \"" << this->GetName() << "\"\n");\
                return false;                                               \
            }                                                               \
            delete mit->second;                                             \
            fMap.erase(mit);                                                \
        }                                                                   \
        else {                                                              \
            if (fKeysLocked) {                                              \
                DbiWarn(  "Set: Keys are locked - not adding `"               \
                          << key << "' to registry \"" << this->GetName() << "\"\n");\
                return false;                                               \
            }                                                               \
        }                                                                   \
        TDbiRegistryItem* ri = new TDbiRegistryItemXxx< TYPE >(new TYPE (val));     \
        fMap[key] = ri;                                                     \
        this->SetDirty();                                                   \
        return true;                                                        \
    }
//REGISTRY_SET(bool)
REGISTRY_SET(char)
REGISTRY_SET(int)
REGISTRY_SET(double)
REGISTRY_SET(TDbiRegistry)
#undef REGISTRY_SET


// Must treat char* special
bool CP::TDbiRegistry::Set(const char* key, const char* val) {
    tRegMap::iterator mit = fMap.find(key);
    if (mit != fMap.end()) {    // Found it
        if (fValuesLocked) {
            DbiWarn("Set: Values are locked - not overwriting `"
                    << key << "\" with \"" << val << "\" in registry \"" << this->GetName() << "\"\n");
            return false;
        }
        if (! dynamic_cast<CP::TDbiRegistryItemXxx<const char*>*>(mit->second)) {
            DbiWarn("Set: attempt to overwrite old value for key \""
                    << key << "\" with different type value "
                    << val << " in registry \"" << this->GetName() << "\"\n");
            return false;
        }
        delete mit->second;
        fMap.erase(mit);
    }
    else {                      // didn't find it
        if (fKeysLocked) {
            DbiWarn("CP::TDbiRegistry::Set: Keys are locked - not adding `"
                    << key << "' in registry \"" << this->GetName() << "\"\n");
            return false;
        }
    }

    char** cpp = new char*;
    (*cpp) = new char [strlen(val)+1];
    strcpy(*cpp,val);
    const char** ccpp = const_cast<const char**>(cpp);
    TDbiRegistryItem* ri = new TDbiRegistryItemXxx< const char* >(ccpp);
    fMap[key] = ri;
    this->SetDirty();
    return true;
}


#define REGISTRY_GET(TYPE)                                      \
    bool CP::TDbiRegistry::Get(const char* key, TYPE & val) const           \
    {                                                               \
        tRegMap::const_iterator mit = fMap.find(key);               \
        if (mit == fMap.end()) return false;                        \
        TDbiRegistryItemXxx<TYPE>* rix =                                \
                dynamic_cast<CP::TDbiRegistryItemXxx<TYPE>*>(mit->second);      \
        if (rix == 0){                                              \
            DbiSevere( "Key " << key             \
                       << " does not have type "    \
                       << #TYPE << " as required"   \
                       << "  ");                     \
            return false;                                             \
        }                                                           \
        val = *(rix->Get());                                        \
        return true;                                                \
    }
//REGISTRY_GET(bool)
REGISTRY_GET(char)
REGISTRY_GET(TDbiRegistry)
REGISTRY_GET(const char*)
REGISTRY_GET(int)
//REGISTRY_GET(double)
bool CP::TDbiRegistry::Get(const char* key, double& val) const {
    tRegMap::const_iterator mit = fMap.find(key);
    if (mit == fMap.end()) {
        return false;
    }
    // try correct type
    TDbiRegistryItemXxx<double>* rixd =
        dynamic_cast<CP::TDbiRegistryItemXxx<double>*>(mit->second);
    if (rixd) {
        val = *(rixd->Get());
        return true;
    }
    // try int
    TDbiRegistryItemXxx<int>* rixi =
        dynamic_cast<CP::TDbiRegistryItemXxx<int>*>(mit->second);
    if (rixi) {
        val = *(rixi->Get());
        return true;
    }
    DbiSevere("Key " << key
              << " does not have type double or int"
              << " as required" << "  ");
    return false;
}

#define REGISTRY_GET_TYPE(NAME, RETTYPE, TYPE)                            \
    RETTYPE CP::TDbiRegistry::Get##NAME(const char* key) const                        \
    {                                                                         \
        TYPE retval = 0;                                                      \
        if (Get(key,retval)) return retval;                                   \
        if (fErrorHandler) { fErrorHandler(); return 0; }                     \
        else {                                                                \
            DbiWarn(  "\nCP::TDbiRegistry::GetTYPE: failed to get value for key \""      \
                      << key << "\" from TDbiRegistry \"" << this->GetName()            \
                      << "\".  Aborting\n\n");                                       \
            bool must_get_a_value = false;                                    \
            assert(must_get_a_value);                                         \
            return 0;                                                         \
        }                                                                     \
    }

//REGISTRY_GET_TYPE(Bool, bool, bool)
REGISTRY_GET_TYPE(Char, char, char)
REGISTRY_GET_TYPE(CharString, const char*, const char*)
REGISTRY_GET_TYPE(Int, int, int)
REGISTRY_GET_TYPE(Double, double, double)
//REGISTRY_GET_TYPE(TDbiRegistry, TDbiRegistry, TDbiRegistry)
#undef REGISTRY_GET_TYPE
CP::TDbiRegistry CP::TDbiRegistry::GetTDbiRegistry(const char* key) const {
    TDbiRegistry retval;
    if (Get(key,retval)) {
        return retval;
    }
    if (fErrorHandler) {
        fErrorHandler();
        return retval;
    }
    else {
        DbiWarn("\nCP::TDbiRegistry::GetTYPE: failed to get value for key \""
                << key << "\" from TDbiRegistry \"" << this->GetName()
                << "\".  Aborting\n\n");
        bool must_get_a_value = false;
        assert(must_get_a_value);
        return retval;
    }
}

const type_info& CP::TDbiRegistry::GetType(const char* key) const {
    tRegMap::const_iterator mit = fMap.find(key);
    if (mit == fMap.end()) {
        return typeid(void);
    }
    return mit->second->GetType();
}
std::string CP::TDbiRegistry::GetTypeAsString(const char* key) const {
    tRegMap::const_iterator mit = fMap.find(key);
    if (mit == fMap.end()) {
        return "void";
    }
    return mit->second->GetTypeAsString();
}

std::string CP::TDbiRegistry::GetValueAsString(const char* key) const {
    std::ostringstream os;
    tRegMap::const_iterator mit = fMap.find(key);
    if (mit == fMap.end()) {
        return "";
    }
    mit->second->PrintStream(os);
    return os.str();
}

void CP::TDbiRegistry::Streamer(TBuffer& b) {
    int nobjects;

    if (b.IsReading()) {
        Version_t v = b.ReadVersion();
        if (v) {}
        TNamed::Streamer(b);

        b >> nobjects;

        for (int i = 0; i < nobjects; ++i) {

            char tmp[1024];
            b >> tmp;
            std::string key(tmp);

            TDbiRegistryItem* ri;
            b >> ri;

            // This is copied from Set(), bad programmer!
            tRegMap::iterator mit = fMap.find(key);
            if (mit != fMap.end()) {
                delete mit->second;
                fMap.erase(mit);
            }
            fMap[key] = ri;

        } // end reading in all TDbiRegistryItems
    } // isReading
    else {
        b.WriteVersion(CP::TDbiRegistry::IsA());
        TNamed::Streamer(b);

        nobjects = fMap.size();
        b << nobjects;

        DbiDebug("Streamer, Writing "<< nobjects <<" objects\n");

        tRegMap::iterator mit = fMap.begin();
        while (mit != fMap.end()) {
            b << mit->first.c_str();

            DbiDebug(mit->first.c_str() << "  ");

            b << mit->second;

            ++mit;
        }
    }
}


std::ostream& CP::TDbiRegistry::PrintStream(std::ostream& os) const {
    os << "['" << this->GetName() << "'";

    tRegMap::const_iterator mit, done = fMap.end();
    for (mit = fMap.begin(); mit != done; ++mit) {
        os << " '" << mit->first << "'=(";
        os << mit->second->GetTypeAsString();
        os << ")";
        mit->second->PrintStream(os);
    }

    os << "]";
    return os;
}


static std::istream& bail(std::istream& is) {
    DbiWarn("CP::TDbiRegistry::Read(istream&) stream corrupted\n");
    return is;
}

std::istream& CP::TDbiRegistry::ReadStream(std::istream& is) {
    TDbiRegistry reg;

    char c;
    if (!is.get(c)) {
        return bail(is);
    }
    if (c != '[') {
        is.putback(c);
        return bail(is);
    }
    std::string name = Util::read_quoted_string(is);
    reg.SetName(name.c_str());

    while (is.get(c)) {
        if (isspace(c)) {
            continue;
        }
        if (c == ']') {
            *this = reg;
            return is;
        }
        is.putback(c);

        // get the key
        std::string key = CP::Util::read_quoted_string(is);
        if (key == "") {
            return bail(is);
        }

        // skip the "="
        if (!is.get(c)) {
            return bail(is);
        }

        // get the "("
        if (!is.get(c) || c != '(') {
            is.putback(c);
            return bail(is);
        }

        // get the type
        std::string type;
        while (is.get(c)) {
            if (c == ')') {
                break;
            }
            type += c;
        }

        // factory:
        TDbiRegistryItem* ri = 0;
        if (type == "char") {
            ri = new TDbiRegistryItemXxx<char>();
        }
        else if (type == "int") {
            ri = new TDbiRegistryItemXxx<int>();
        }
        else if (type == "double") {
            ri = new TDbiRegistryItemXxx<double>();
        }
        else if (type == "string") {
            ri = new TDbiRegistryItemXxx<const char*>();
        }
        else if (type == "TDbiRegistry") {
            ri = new TDbiRegistryItemXxx<CP::TDbiRegistry>();
        }
        else {
            return bail(is);
        }

        ri->ReadStream(is);
        reg.fMap[key] = ri;
    }
    return is;

}

