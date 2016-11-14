;/* --------------------------------------------------------
; HEADER SECTION
;*/
SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )
;
;
FacilityNames=(System=0x0:FACILITY_SYSTEM
               Runtime=0x2:FACILITY_RUNTIME
               Stubs=0x3:FACILITY_STUBS
               Io=0x4:FACILITY_IO_ERROR_CODE
              )
LanguageNames=(Russian=0x419:MSG00419)
;
;/* ------------------------------------------------------------------
; MESSAGE DEFINITION SECTION
;*/

MessageIdTypedef=DWORD

MessageId=0x100
Severity=Error
Facility=System
SymbolicName=MSG_ERROR_EVENT_TEXT
Language=Russian
Error: %1
.

MessageId=0x101
Severity=Informational
Facility=System
SymbolicName=MSG_INFORMATIONAL_EVENT_TEXT
Language=Russian
Information: %1
.

MessageId=0x102
Severity=Warning
Facility=System
SymbolicName=MSG_WARNING_EVENT_TEXT
Language=Russian
Warning: %1
.
