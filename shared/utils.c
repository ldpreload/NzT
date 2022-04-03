#include <Windows.h>
#include <stdio.h>
#include <time.h>

#include "utils.h"
#include "guid.h"
#include "globals.h"
#include "crypto.h"
#include "error.h"

ULONG g_MachineRandomSeed = 0;

PCHAR GenerateRandomLetters(size_t Length)
{
	const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789123456789";
	POINT Point;
	PCHAR String;

	GetCursorPos(&Point);

	INT Seed = Point.x * Point.y;

	srand(Seed);

	String = malloc(sizeof(char) * (Length + 1));

	if (Length) {
		for (size_t n = 0; n < Length; n++) {
			int key = rand() % (int)(sizeof charset - 1);
			String[n] = charset[key];
		}
		String[Length] = '\0';
	}

	return String;
}

DWORD GetOperatingSystem()
{
	DWORD dwOS = 0;
	OSVERSIONINFOEX Version;

	//memset(&Version, 0, sizeof(OSVERSIONINFOEXW));
	Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (GetVersionEx(&Version))
	{
		//printf("%d %d\n", Version.dwMajorVersion, Version.dwMinorVersion);
		if (Version.wProductType == VER_NT_WORKSTATION)
		{
			if (Version.dwMajorVersion == 5 && Version.dwMinorVersion == 0)
				dwOS = WINDOWS_2000;
			else if (Version.dwMajorVersion == 5 && Version.dwMinorVersion > 0)
				dwOS = WINDOWS_XP;
			else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 0)
				dwOS = WINDOWS_VISTA;
			else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 1)
				dwOS = WINDOWS_7;
			else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 2)
				dwOS = WINDOWS_8;
			else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 3)
				dwOS = WINDOWS_8_1;
			else if (Version.dwMajorVersion == 10)
				dwOS = WINDOWS_10;
		}
		else if (Version.wProductType == VER_NT_DOMAIN_CONTROLLER ||
			Version.wProductType == VER_NT_SERVER)
		{
			if (Version.dwMajorVersion == 5 && Version.dwMinorVersion == 2)		 dwOS = WINDOWS_SERVER_2003;
			else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 0) dwOS = WINDOWS_SERVER_2008;
			else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 1) dwOS = WINDOWS_SERVER_2008;
			else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 2
			|| Version.dwMinorVersion == 3)										 dwOS = WINDOWS_SERVER_2012;
			else if (Version.dwMajorVersion == 10)								 dwOS = WINDOWS_SERVER_2016;
		}
	}

	return dwOS;
}

PCHAR GetOperatingSystemString(DWORD OS)
{
	switch (OS)
	{
	case WINDOWS_2000:
		return "Windows 2000";
	case WINDOWS_XP:
		return "Windows XP";
	case WINDOWS_VISTA:
		return "Windows Vista";
	case WINDOWS_7:
		return "Windows 7";
	case WINDOWS_8:
		return "Windows 8";
	case WINDOWS_8_1:
		return "Windows 8.1";
	case WINDOWS_10:
		return "Windows 10";
	default:
		return "Unknown";
	}
}