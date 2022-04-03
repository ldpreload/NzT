#ifndef _GUID_H_
#define _GUID_H_

#define	GUID_STR_LENGTH		16*2+4+2

#define szGuidStr_src		("%08X%04X")

typedef union _GUID_EX
{
	GUID Guid;
	struct
	{
		ULONG Data1;
		ULONG Data2;
		ULONG Data3;
		ULONG Data4;
	};
}GUID_EX, *PGUID_EX;

VOID	GenGuid(GUID* pGuid, PULONG pSeed);
LPTSTR	GenGuidName(PULONG pSeed, LPTSTR Prefix OPTIONAL, LPTSTR Postfix OPTIONAL);
VOID	FillGuidName(PULONG	pSeed, LPTSTR pGuidName);
LPTSTR	GuidToString(GUID* pGuid);
ULONG	GuidToBuffer(GUID* pGuid, LPTSTR pBuffer);

#endif