#include <Windows.h>
#include <stdio.h>

#include "log.h"

LOG_FORMAT_S LOG_FORMAT[] =
{
	{PEER_JOIN,	      "Join: ",	     FOREGROUND_GREEN},
	{PEER_QUIT,	      "Quit: ",   FOREGROUND_RED},
	{PEER_TIMEOUT,	  "Timed-out: ", FOREGROUND_RED},
	{SERVER_MESSAGE,  "Server: ",    FOREGROUND_BLUE},
	{NONE_ERROR,	  "Error: ",      FOREGROUND_RED}
};

VOID SetConsoleFontColor(USHORT Color)
{
	HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(Handle, Color);
	return;
}

VOID ConsoleOutput(MESSAGE Message, PCHAR Format, ...)
{
	SYSTEMTIME Time;
	PCHAR Buffer;
	ULONG Size;
	va_list VA;

	if (!(Buffer = malloc(512)))
		return;

	GetLocalTime(&Time);

	for (int i = 0; LOG_FORMAT[i].Message != 0; i++)
	{
		if (LOG_FORMAT[i].Message == Message)
		{
			SetConsoleFontColor(LOG_FORMAT[i].Color);
			break;
		}
	}

	va_start(VA, Format);
	Size = wsprintfA(Buffer, "[%02u:%02u:%02u] ", Time.wHour, Time.wMinute, Time.wSecond);
	vsprintf(Buffer + Size, Format, VA);
	va_end(VA);

	printf("%s\n", Buffer);

	free(Buffer);
	SetConsoleFontColor(0x0007);
}