#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define G_SYSTEM_VERSION		1		// OS version
#define	G_CURRENT_PROCESS_ID	2		// Current process ID
#define G_CURRENT_PROCESS_PATH	8		// Current process module full path (for both DLL and EXE)

// Global process flags
#define	GF_WOW64_PROCESS		1	
#define	GF_ADMIN_PROCESS		2

// Global variables
extern	DWORD	g_SystemVersion;
extern	DWORD	g_CurrentProcessId;
extern	LPTSTR	g_CurrentProcessPath;

VOID		ReleaseGlobals(VOID);
WINERROR	InitGlobals(HMODULE CurrentModule, ULONG Flags);

#endif