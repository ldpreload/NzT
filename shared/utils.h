#pragma once

typedef enum
{
	WINDOWS_2000        = 0,
	WINDOWS_XP			= 1,
	WINDOWS_VISTA		= 2,
	WINDOWS_7			= 3,
	WINDOWS_8			= 4,
	WINDOWS_8_1			= 5,
	WINDOWS_10			= 6,
	WINDOWS_SERVER_2000 = 7,
	WINDOWS_SERVER_2003 = 8,
	WINDOWS_SERVER_2008 = 9,
	WINDOWS_SERVER_2012 = 10,
	WINDOWS_SERVER_2016 = 11
} OS_LIST;

typedef enum
{
	ARCHITECTURE_X86 = 0,
	ARCHITECTURE_x64 = 1
} ARCHITECTURE;

typedef struct
{
	USHORT	OS;
	PCHAR	Username;
	BOOL	Is64Bit;
} PC_INFO, *PPC_INFO;

char *GenerateRandomLetters(size_t Length);
DWORD GetOperatingSystem();
PCHAR GetOperatingSystemString(DWORD OS);