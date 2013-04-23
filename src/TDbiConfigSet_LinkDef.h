#ifdef __CINT__
#pragma link C++ class CP::TDbiConfigSet;
#pragma link C++ class CP::TDbiResultSetHandle<CP::TDbiConfigSet>+;
#pragma link C++ class CP::TDbiWriter<CP::TDbiConfigSet>+;

// Normally this would be part of CP::TDbiLogEntry_LinkDef but TDbiWriter.hxxas a 
// DBiLogEntry and this confuses ROOT's dictionary generator.

#pragma link C++ class CP::TDbiWriter<CP::TDbiLogEntry>+;

#pragma link C++ class CP::TDbiResultSetHandle<CP::TDbiValidityRec>+;

#endif
