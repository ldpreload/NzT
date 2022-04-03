#include <Windows.h>
#include <stdio.h>
#include <crtdbg.h>
#include <winsock.h>

#include "guid.h"


VOID GenGuid(GUID * pGuid, PULONG pSeed)
{
	pGuid->Data1 = rand();
	pGuid->Data2 = (USHORT)rand();
	pGuid->Data3 = (USHORT)rand();

	for (ULONG i = 0; i < 8; i++)
		pGuid->Data4[i] = (UCHAR)rand();
}

VOID FillGuidName(
	IN OUT PULONG pSeed,
	OUT	   LPTSTR pGuidName)
{
	GUID	Guid;
	ULONG	Size;

	GenGuid(&Guid, pSeed);
	Size = wsprintf(pGuidName, szGuidStr_src, *(ULONG*)&Guid.Data4[2], *(USHORT*)&Guid.Data4[6]);
	_ASSERT(Size <= GUID_STR_LENGTH);
}

ULONG GuidToBuffer(
	GUID*	pGuid,
	LPTSTR	pBuffer
)
{
	return (wsprintf(pBuffer, szGuidStr_src, htonl(*(ULONG*)&pGuid->Data4[2]), htons(*(USHORT*)&pGuid->Data4[6])));
}

LPTSTR GuidToString(GUID *pGuid)
{
	LPTSTR pGuidString;

	if (pGuidString = (LPTSTR)malloc((GUID_STR_LENGTH + 1) * sizeof(TCHAR)))
		GuidToBuffer(pGuid, pGuidString);

	return (pGuidString);
}

LPTSTR GenGuidName(
	IN OUT PULONG	pSeed,
	IN	   LPTSTR	Prefix	OPTIONAL,
	IN     LPTSTR	Postfix OPTIONAL
)
{
	ULONG	Length = GUID_STR_LENGTH + 1;
	LPTSTR	GuidString, Name = NULL;
	GUID	Guid;

	GenGuid(&Guid, pSeed);
	if (GuidString = GuidToString(&Guid))
	{
		if (Prefix)
			Length += strlen(Prefix);
		if (Postfix)
			Length += strlen(Postfix);

		if (Name = (LPTSTR)malloc(Length * sizeof(TCHAR)))
		{
			Name[0] = 0;

			if (Prefix)
				strcpy(Name, Prefix);

			strcat(Name, GuidString);
			
			if (Postfix)
				strcat(Name, Prefix);
		}

		free(GuidString);
	}

	return (Name);
}