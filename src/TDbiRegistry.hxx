/**
 *
 *
 *
 * \class CP::TDbiRegistry
 *
 *
 * \brief Type safe heterogenous collection of key value pairs.
 *
 * Stored in a std::map, keyed by a string. Values can be int, char, string, double or TDbiRegistryItem.
 * Actuall all stored internally as TDbiRegistryItem s
 * Contact: bv@bnl.gov
 *
 * Created on: Wed Oct 25 17:13:16 2000
 *
 */

#ifndef TDBIREGISTRY_H
#define TDBIREGISTRY_H

#include <TNamed.h>

#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

class TDbiRegistryItem;

#include "TDbiRegistryItemXxx.hxx"    //Needed for LinkDef

namespace CP {
    class TDbiRegistry : public TNamed {
    public:
        typedef std::map<std::string,TDbiRegistryItem*> tRegMap;
        typedef void (*ErrorHandler)(void);

        /// Create a TDbiRegistry.  If readonly is false, any key's value can
        /// be set multiple times, otherwise only the first setting is allowed.
        /// See methods below regarding locking of keys and values.
        explicit TDbiRegistry(bool readonly = true);

        /// Deep copy constructor.
        TDbiRegistry(const TDbiRegistry& rhs);

        virtual ~TDbiRegistry();

        /// Deep assignment.
        TDbiRegistry& operator=(const TDbiRegistry& rhs);

        /// Copy rhs into this, respects this's locks.
        void Merge(const TDbiRegistry& rhs);

        /// Return number of entries.
        unsigned int Size() const {
            return fMap.size();
        }

        /// Check if key exists.
        bool KeyExists(const char* key) const;
        void RemoveKey(const char* key);

        /// Clear TDbiRegistry - deletes all items.
        void Clear(Option_t* option=""); //*MENU*

        /// Dump to cerr.
        void Dump(void) const;      //*MENU*

        /// Print to cout (without extraneous bits of Dump()).
        virtual std::ostream& PrintStream(std::ostream& os) const;
        virtual std::istream& ReadStream(std::istream& is);

        // TObject::Print().
        virtual void Print(Option_t* option="") const; //*MENU*
        virtual std::ostream& PrettyPrint(std::ostream& os) const;

        // TObject::Browse()
        /// The default implementation crashes TBrowser, doing nothing is better.
        virtual void Browse(TBrowser*) {}

        /// Control if an existing value can be set.
        virtual bool ValuesLocked(void) const {
            return fValuesLocked;
        }
        /// Control if an existing value can be set.
        virtual void LockValues(void) {
            fValuesLocked = true;   //*MENU*
        }
        /// Control if an existing value can be set.
        virtual void UnLockValues(void) {
            fValuesLocked = false;   //*MENU*
        }

        /// Control if new key/value pairs can be added.
        virtual bool KeysLocked(void) const {
            return fKeysLocked;
        }
        /// Control if new key/value pairs can be added.
        virtual void LockKeys(void) {
            fKeysLocked = true;   //*MENU*
        }
        /// Control if new key/value pairs can be added.
        virtual void UnLockKeys(void) {
            fKeysLocked = false;   //*MENU*
        }

        /// Access an internal "dirty" flag TDbiRegistry maintains (but does
        /// not use) It will be set any time a non-const method is
        /// accessed, or explicitly via SetDirty().  Initially a TDbiRegistry
        /// is dirty (original sin?).
        void SetDirty(bool is_dirty = true) {
            fDirty = is_dirty;
        }
        bool IsDirty() {
            return fDirty;
        }


        void SetErrorHandler(ErrorHandler eh) {
            fErrorHandler = eh;
        }

//    bool Get(const char* key, bool& b) const;
        /// Access a char value.  Return true and set second argument if key is
        /// found, else returns false.
        bool Get(const char* key, char& c) const;
        /// Access a string value.  Return true and set second argument if key is
        /// found, else returns false.
        bool Get(const char* key, const char*& s) const;
        /// Access an int value.  Return true and set second argument if key is
        /// found, else returns false.
        bool Get(const char* key, int& i) const;
        /// Access a double  value.  Return true and set second argument if key is
        /// found, else returns false.
        bool Get(const char* key, double& d) const;
        /// Access a TDbiRegistry value.  Return true and set second argument if key is
        /// found, else returns false.
        bool Get(const char* key, TDbiRegistry& r) const;

        /// Return the type_info of the value corresponding to the given
        /// key.  If key doesn't exist, type_info for type void is returned.
        const std::type_info& GetType(const char* key) const;
        /// Return "int", "double", "char", "string", "TDbiRegistry" or "void"
        std::string GetTypeAsString(const char* key) const;
        /// see format.txt
        std::string GetValueAsString(const char* key) const;



//    bool        GetBool(const char* key) const;
/// Access a char value.  Returns value if key lookup succeeds, else
        /// prints warning message.  Use above Get() methods for a safer
        /// access method.
        char        GetChar(const char* key) const;
        /// Access a string value.  Returns value if key lookup succeeds, else
        /// prints warning message.  Use above Get() methods for a safer
        /// access method.
        const char* GetCharString(const char* key) const;
        /// Access an int value.  Returns value if key lookup succeeds, else
        /// prints warning message.  Use above Get() methods for a safer
        /// access method.
        int         GetInt(const char* key) const;
        /// Access a double value.  Returns value if key lookup succeeds, else
        /// prints warning message.  Use above Get() methods for a safer
        /// access method.
        double      GetDouble(const char* key) const;
        /// Access an CP::TDbiRegistry value.  Returns value if key lookup succeeds, else
        /// prints warning message.  Use above Get() methods for a safer
        /// access method.
        TDbiRegistry    GetTDbiRegistry(const char* key) const;
//    bool Set(const char* key, bool b);
        /// Set the value associated with the given key.  Return false if
        /// locks prevent setting or if type mismatch.

        bool Set(const char* key, char c);
        bool Set(const char* key, const char* s);
        bool Set(const char* key, int i);
        bool Set(const char* key, double d);
        bool Set(const char* key, TDbiRegistry r);
/// Container for a registry key. Contains a std::map of key value pairs where key
/// is a string and value is a registry item. Also stores pointer to a TDbiRegistry object.
        class TDbiRegistryKey {

        public:
            TDbiRegistryKey();
            TDbiRegistryKey(const TDbiRegistry* r);
            virtual ~TDbiRegistryKey();

            const char* operator()(void);

        private:

            const TDbiRegistry* fReg;
            std::map<std::string,TDbiRegistryItem*>::iterator fIt;
        };                              // end of class TDbiRegistryKey

        TDbiRegistryKey Key(void) const;

    private:
        bool fValuesLocked;
        bool fKeysLocked;
        ErrorHandler fErrorHandler;  //! not written out
#ifndef __CINT__
        friend class TDbiRegistryKey;
//    template<class T> bool SetPtr(std::string key, T* val);
        tRegMap fMap;
#endif
        bool fDirty;

        ClassDef(CP::TDbiRegistry,1)
    };                              // end of class TDbiRegistry
}

inline std::ostream& operator<<(std::ostream& os, const CP::TDbiRegistry& r) {
    return r.PrintStream(os);
}


#include "TDbiRegistryItemXxx.hxx"

#endif  // TDBIREGISTRY_H
