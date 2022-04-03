#include <Windows.h>
#include <winsock.h>
#include <stdio.h>

#include "socket.h"

/*
*	SocketInitialize: Initialize Winsock
*/
INT SocketInitialize()
{ 
	WSADATA wsa;
	if (WSAStartup(0x202, &wsa) != 0)
		return -1;
	return 0;
}

/*
*	SocketConnect: Send data by length to target socket file descriptor
*		@Socket - Target socket file descriptor
*		@Host   - Target host
*		@Port   - Target port
*/
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

/*
*	SocketReadData: Receive actual data
*		@Socket - Target socket file descriptor
*		@Data   - Pointer to data context structure
*/
WINERROR SocketReadData(
	SOCKET			Socket,
	PDATA_CONTEXT   Ctx
)
{
	SHORT Read;

	Read = recv(Socket, Ctx->Data.Message + Ctx->Data.Length, Ctx->DataLength, 0);
	if (Read <= 0)
		return CONNECTION_DEAD;

	Ctx->Data.Length += Read;

	if (Ctx->Data.Length == Ctx->DataLength)
		return CONNECTION_ALIVE;

	//data length comparison failed
	return ERROR_UNSUCCESSFULL;
}


/*
*	SocketReadLength: Receive exact amount of incoming data
*		@Socket - Target socket file descriptor
*		@Data   - Pointer to data context structure
*/
WINERROR SocketReadLength(
	SOCKET			Socket,
	PDATA_CONTEXT   Ctx
)
{
	SHORT Read;

	//receive the length of incomming data
	if (Ctx->BufferLength == 0)
		Read = recv(Socket, Ctx->Buffer, 2, 0);
	else
		Read = recv(Socket, &Ctx->Buffer[1], 1, 0);

	if (Read <= 0)
		return CONNECTION_DEAD;

	//check for atleast 2 bytes then receive real data
	Ctx->BufferLength += Read;
	if (Ctx->BufferLength == 2)
	{
		Ctx->DataLength = *(USHORT *)Ctx->Buffer;
		if (Ctx->DataLength == 0)
		{
			//no new data
			Ctx->BufferLength = 0;
			return CONNECTION_NN;
		}

		//allocate data
		Ctx->Data.Message = (PCHAR)malloc(Ctx->DataLength);
		if (Ctx->Data.Message == NULL)
			return ERROR_NOT_ENOUGH_MEMORY;
	}

	return NO_ERROR;
}

/*
*	SocketRead: Receive data
*		@Socket - Target socket file descriptor
*		@Data   - Pointer to data context structure
*/
BOOL SocketRead(
	SOCKET			Socket,
	PDATA_CONTEXT   Ctx
)
{
	fd_set FD;

	FD_ZERO(&FD);
	FD_SET(Socket, &FD);

	//poll fd for data
	if (select(0, &FD, NULL, NULL, NULL) == SOCKET_ERROR)
	{
		printf("Error: select() (%d)\n", GetLastError());
		return FALSE;
	}

	if (Ctx->DataLength == 0)
		return SocketReadLength(Socket, Ctx);
	else
		return SocketReadData(Socket, Ctx);
}


/*
*	SocketSend: Send data by length to target socket file descriptor
*		@Socket - Target socket file descriptor
*		@Buffer - Data to be sent
*		@Length - Exact length of data
*/
BOOL SocketSend(
	SOCKET	Socket,
	PCHAR	Buffer,
	USHORT Length
)
{
	//send length of data
	if (send(Socket, (PCHAR)&Length, 2, 0) == SOCKET_ERROR)
		return FALSE;

	if (send(Socket, Buffer, Length, 0) == SOCKET_ERROR)
		return FALSE;

	return TRUE;
}

/*
*	SocketResetData: Reset socket data context structure
*		@Data - Pointer to INCOMING_DATA context structure
*/
VOID SocketResetData(PDATA_CONTEXT Ctx)
{
	if (Ctx->Data.Message == NULL)
		return;

	if (Ctx->DataLength > 0)
	{
		free(Ctx->Data.Message);
		Ctx->Data.Length = 0;
		Ctx->DataLength = 0;
	}

	Ctx->BufferLength = 0;
}

/*
*	SocketShutdown: Shutdown and clean-up socket
*		@Socket - Target socket file descriptor
*		@Data	- Pointer to INCOMING_DATA context structure
*/
VOID SocketShutdown(SOCKET Socket, PDATA_CONTEXT Data)
{
	SocketResetData(Data);
	closesocket(Socket);
}