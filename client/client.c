#include <Windows.h>
#include <stdio.h>
#include <winsock.h>

#include "..\shared\socket.h"
#include "..\shared\config.h"
#include "..\shared\utils.h"

#define CMD_TEST "CMD_TEST"

typedef struct _condata_s
{
	int sock;
	DATA_CONTEXT Ctx;
	int connected;
	PCHAR Id;
} condata_s;

void parse(condata_s *cd, PCHAR Message, int Size)
{
	SocketResetData(&cd->Ctx);

	return;
}

VOID TestThread(VOID)
{
	CHAR Buffer[128];
	struct sockaddr_in addr;
	condata_s cd;
	int r;

	memset(Buffer, 0, sizeof(Buffer));
	memset(&cd, 0, sizeof(condata_s));

	cd.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (cd.sock < 0)
	{
		printf("Error: failed to initalize socket (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	//TODO resolve DNS to IP address, and command-line argument
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(atoi("1337"));
	addr.sin_family = AF_INET;

	if (connect(cd.sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
	{
		printf("Error: failed to connect (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	cd.connected = 1;

	wsprintf(Buffer, "%s\nPC_INFO\n%d\n", BOT_CODE, GetOperatingSystem());
	SocketSend(cd.sock, Buffer, (USHORT)strlen(Buffer) + 1);

	while (cd.connected)
	{
		r = SocketRead(cd.sock, &cd.Ctx);
		if (r == CONNECTION_ALIVE)
		{
			parse(&cd, cd.Ctx.Data.Message, cd.Ctx.Data.Length);
			SocketResetData(&cd.Ctx);
		}
		else if (r == CONNECTION_DEAD)
		{
			cd.connected = 0;
			break;
		}
	}

	return;
}

int __cdecl main(int argc, char *argv[])
{
	WSADATA			WsaData;

	if ((WSAStartup(0x202, &WsaData)))
		return -1;

	for (int i = 0; i < 1; i++)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TestThread, NULL, 0, 0);
		Sleep(10);
	}

	getchar();
	return 0;
}

/*int __cdecl main(int argc, char *argv[])
{
	WSADATA			WsaData;
	
	if ((WSAStartup(0x202, &WsaData)))
		return;

	struct sockaddr_in addr;
	condata_s cd;
	int r;

	memset(&cd, 0, sizeof(condata_s));

	cd.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (cd.sock < 0)
	{
		printf("Error: failed to initalize socket (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	//TODO resolve DNS to IP address, and command-line argument
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(atoi("1337"));
	addr.sin_family = AF_INET;

	if (connect(cd.sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
	{
		printf("Error: failed to connect (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	cd.connected = 1;

	SocketSend(cd.sock, BOT_CODE, strlen(BOT_CODE) + 1);

	while (cd.connected)
	{
		r = SocketRead(cd.sock, &cd.data);
		if (r == CONNECTION_ALIVE)
		{
			parse(&cd, cd.data.Ctx.pData, cd.data.Ctx.Length);
			SocketResetData(&cd.data);
		}
		else if (r == CONNECTION_DEAD)
		{
			cd.connected = 0;
			break;
		}
	}

	printf("Connection terminated\n");
	closesocket(cd.sock);
	SocketResetData(&cd.data);

	exit(EXIT_SUCCESS);
}
*/