////////////////////////////////////////////////////////////////////////
// $Id: TDbiCfgDialog.cxx,v 1.1 2011/01/18 05:49:19 finch Exp $
//
// Default implementation for a text-based dialog with a user to fill
// a TDbiRegistry object.
//
// messier@huhepl.harvard.edu
////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "TDbiCfgDialog.hxx"

//......................................................................

CP::TDbiCfgDialog::TDbiCfgDialog() :
    fCurrent(),
    fDefault(),
    fResult()
{ }

//......................................................................

CP::TDbiCfgDialog::TDbiCfgDialog(const TDbiRegistry& cur, const TDbiRegistry& defl) :
    fCurrent(cur),
    fDefault(defl),
    fResult()
{ }

//......................................................................

CP::TDbiCfgDialog::~TDbiCfgDialog() { }

//......................................................................
//======================================================================
/// Copy the registry r to the set of values to display as current
/// values
//======================================================================
void CP::TDbiCfgDialog::SetCurrent(const TDbiRegistry& r) {

    fCurrent.UnLockValues();
    fCurrent = r;
    fCurrent.LockValues();
}

//......................................................................
//======================================================================
/// Copy the registry r to the set of values to display as default values
//======================================================================
void CP::TDbiCfgDialog::SetDefault(const TDbiRegistry& r) {

    fDefault.UnLockValues();
    fDefault = r;
    fCurrent.LockValues();
}

//......................................................................
//======================================================================
/// Conduct the dialog with the user to load new values into the
/// registry. Return the registry stuffed with the new values.
/// Can get one of either char, string, int or double.
//======================================================================
CP::TDbiRegistry& CP::TDbiCfgDialog::Query() {

    TDbiRegistry::TDbiRegistryKey rk = fCurrent.Key();

    // Setup result registry
    fResult = fCurrent;
    fResult.UnLockValues();

    const char* k; // Key name
    while ((k=rk())) {
        // bool      b; // Temp. bool value
        char            c; // Temp. char value
        const char*     s; // Temp. string value
        int             i; // Temp. int value
        double          d; // Temp. double value
        TDbiRegistry        r; // Temp. TDbiRegistry value

        // Use the 'current' registry to divine the types of junk...
        // Currently there's no good way to do this and TDbiRegistry spits
        // warnings at you for even trying...
        bool isBool   = false;
        // bool isBool   = fDefault.Get(k, b);
        bool isChar   = fDefault.Get(k, c);
        bool isString = fDefault.Get(k, s);
        bool isInt    = fDefault.Get(k, i);
        bool isDouble = fDefault.Get(k, d);
        bool isTDbiRegistry = false;
        // bool isTDbiRegistry = fCurrent.Get(k, r);

#define PROMPT(t,c,d) \
    std::cout << " "<<t<<" "<<k<<" = ["<<d<<"] "<<c<<" =? ";
        // OK, now get the user's input. One "if" per type...
        if (isBool) {
            // bool b1, b_in;
            // fDefault.Get(k, b1);
            // PROMPT("bool",b,b1);
            // std::cin >> b_in;
            // fResult.Set(k, d_in);
        }
        else if (isChar) {
            char c1, c_in;
            fDefault.Get(k, c1);
            PROMPT("char",c,c1);
            std::cin >> c_in;
            fResult.Set(k, c_in);
        }
        else if (isString) {
            const char* s1;
            std::string s_in;
            fDefault.Get(k, s1);
            PROMPT("string",s,s1);
            std::cin >> s_in;
            // This is OK, TDbiRegistry allocates space and stores a copy
            // of the string, not just the pointer
            fResult.Set(k, s_in.c_str());
        }
        else if (isInt) {
            int i1, i_in;
            fDefault.Get(k, i1);
            PROMPT("int",i,i1);
            std::cin >> i_in;
            fResult.Set(k, i_in);
        }
        else if (isDouble) {
            double d1, d_in;
            fDefault.Get(k, d1);
            PROMPT("double",d,d1);
            std::cin >> d_in;
            fResult.Set(k, d_in);
        }
        else if (isTDbiRegistry) {
            // TDbiRegistry r1, r_in;
            // std::string rins;
            // fDefault.Get(k, r1);
            // PROMPT("TDbiRegistry",r,r1);
            // std::cin >> r_ins;
            // r_in << r_ins;
            // fResult.Set(k, r_in);
        }
    } // loop over keys
    return fResult;
}

////////////////////////////////////////////////////////////////////////
