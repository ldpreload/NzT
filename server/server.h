#ifndef _SERVER_H_
#define _SERVER_H_

#include "list.h"
#include "socket.h"

/* Defines */
#define WIN32_LEAN_AND_MEAN

/* Typedefs */
typedef enum
{
	PERMISSION_NONE			= 0,
	PERMISSION_BOT			= 1,
	PERMISSION_MASTER		= 2,
	PERMISSION_VISITOR		= 3
} PERMISSIONS;

typedef enum
{
	STATUS_AUTH_PENDING		=  0,
	STATUS_AUTH_OK			=  1
} STATUS;

typedef enum
{
	NONE_ERROR				=  0,
	PEER_JOIN				=  1,
	PEER_QUIT				=  2,
	PEER_TIMEOUT			=  3,
	PEER_MESSAGE			=  4,
	QUERY_TOTAL				=  5,
	QUERY_LIST_BASIC		=  6,
	QUERY_LIST_ADV			=  7,
	COMMAND_RECONNECT		=  8,
	COMMAND_KILL			=  9,
	COMMAND_MUTE			=  10,
	SERVER_MESSAGE          =  11,
	SERVER_RESTART			=  12,
	SERVER_QUIT				=  13,
	SERVER_INFO				=  14,
	SERVER_PEAKED			=  15
} MESSAGE;

/* Context Structures */
typedef struct _SERVER_CONTEXT_S
{
	CRITICAL_SECTION	ClientListLock;		//critical section for linked list
	LIST_ENTRY			ClientListHead;		//head to linked list
	SOCKET				Socket;				//listening server socket
	SOCKADDR_IN			Address;			//server socket address
	HANDLE				AcceptThread;		//control thread for handling events
	ULONG				NumberOfClients;	//total number of clients connected
	ULONG				NumberOfMasters;	//total number of clients connected
	ULONG				AcceptThreadId;		//server control thread id
	DATA_CONTEXT		DataCtx;			//pointer to data context structure
} SERVER_CONTEXT, *PSERVER_CONTEXT;

typedef struct _CLIENT_CONTEXT_S
{
	LIST_ENTRY			Entry;				//single linked list entry
	PSERVER_CONTEXT		Server;				//pointer to current server structure
	SOCKADDR_IN*		Address;			//pointer to client socket address
	SOCKET				Socket;				//client socket
	PCHAR				Id;					//client identification
	HANDLE				WorkerThread;		//worker thread for handling events
	ULONG				WorkerThreadId;		//worker thread id
	STATUS				Status;				//client status		(STATUS_AUTH_PENDING | STATUS_AUTH_OK)
	PERMISSIONS			Permission;			//client permission (PERMISSION_MASTER   | PERMISSION_BOT)
	DATA_CONTEXT		DataCtx;			//pointer to data context structure
	PCHAR				OS;
} CLIENT_CONTEXT, *PCLIENT_CONTEXT;

/* Functions */
VOID ClientParse(PCLIENT_CONTEXT pClient);
WINERROR ClientAuthenticate(PCLIENT_CONTEXT pClient);
VOID ClientThread(PCLIENT_CONTEXT pClient);

VOID ServerAcceptThread(PSERVER_CONTEXT pServer);
WINERROR ServerStart(USHORT Port);

#endif