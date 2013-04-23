#ifdef __CINT__
#pragma link C++ class ND::TDemo_DB_Table+;

// The TableRow subclass' *_LinkDef.h must request a dictionary for
// the templated ND::TResultSetHandle instantiated on it.  However that
// class inherits from the backend ND::TDbiResultSetHandle so that dictionary is
#ifndef USE_NEW_DBI_API 
#pragma link C++ class ND::TDbiResultSetHandle<ND::TDemo_DB_Table>+;
#else
#pragma link C++ class ND::TDbiResultSetHandle<ND::TDemo_DB_Table>+;
#endif
// also required.

#pragma link C++ class ND::TResultSetHandle<ND::TDemo_DB_Table>+;

#endif
