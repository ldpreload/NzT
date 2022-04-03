#include <Windows.h>
#include <stdio.h>
#include <ShlObj.h>

#include "globals.h"
#include "shared.h"

DWORD		g_SystemVersion			= 0;
DWORD		g_CurrentProcessId		= 0;
LPTSTR		g_CurrentProcessPath	= 0;

VOID ReleaseGlobals(VOID)
{
	if (g_CurrentProcessId)
		free(g_CurrentProcessId);
}

WINERROR InitGlobals(HMODULE CurrentModule, ULONG Flags)
{
	WINERROR Status = NO_ERROR;
	if (Flags & G_CURRENT_PROCESS_ID)
		g_CurrentProcessId = GetCurrentProcessId();

	return (Status);
}