
#pragma once

#pragma warning(disable:4201)  //nonstandard extension used : nameless struct/union

#define INITGUID
#include <ntddk.h>
#include <wdf.h>
#include <ndis.h>
#include <fwpmk.h>
#include <fwpsk.h>
#include <netioapi.h>
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <wdmsec.h> // for SDDLs
