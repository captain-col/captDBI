#ifdef __CINT__
#pragma link C++ class CP::TDemo_DB_Table+;

// The TableRow subclass' *_LinkDef.h must request a dictionary for
// the templated CP::TResultSetHandle instantiated on it.  However that
// class inherits from the backend CP::TDbiResultSetHandle so that dictionary is
#pragma link C++ class CP::TDbiResultSetHandle<CP::TDemo_DB_Table>+;

// also required.
#pragma link C++ class CP::TResultSetHandle<CP::TDemo_DB_Table>+;

#endif
