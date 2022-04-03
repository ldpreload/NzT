#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "error.h"

/* Defines */
#define CONNECTION_DEAD -1
#define CONNECTION_ALIVE 1
#define CONNECTION_NN    0				//nothing new

/* Structures */
typedef struct
{
	PCHAR Message;						//pointer to allocated data
	USHORT Length;						//length of allocated data
} INCOMING_DATA, *PINCOMING_DATA;

typedef struct
{
	INCOMING_DATA Data;					//data structure
	USHORT		  DataLength;			//total length of incoming data
	CHAR	      Buffer[2];			//buffer saving data from incomplete stream
	SHORT		  BufferLength;			//length of saved data
} DATA_CONTEXT, *PDATA_CONTEXT;

/* Functions */

INT SocketInitialize();

static INT SocketConnect(SOCKET Socket, PCHAR Host, PCHAR Port);

WINERROR SocketRead(
	SOCKET			Socket,
	PDATA_CONTEXT  Data
);

WINERROR SocketSend(
	SOCKET	Socket,
	PCHAR	Buffer,
	USHORT	Length
);

VOID SocketResetData(PDATA_CONTEXT Data);

VOID SocketShutdown(SOCKET Socket, PDATA_CONTEXT Data);

#endif