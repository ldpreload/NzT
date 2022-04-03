#include <Windows.h>
#include <stdio.h>

#include "crypto.h"

ULONG Crc32(
	PCHAR pMem,		// data buffer
	ULONG uLen		// length of the buffer in bytes
)
{
	ULONG		i, c;
	ULONG		dwSeed = -1;

	while (uLen--)
	{
		c = *pMem;
		pMem = pMem + 1;

		for (i = 0; i < 8; i++)
		{
			if ((dwSeed ^ c) & 1)
				dwSeed = (dwSeed >> 1) ^ 0xEDB88320;
			else
				dwSeed = (dwSeed >> 1);
			c >>= 1;
		}
	}
	return(dwSeed);
}

VOID XorCrypt(PVOID source, DWORD size, DWORD key)
{
	register LPBYTE src = (LPBYTE)source;
	for (register int i = 0; i < size; i++)
	{
		src[i] ^= key % (i + 1);
	}
}
