#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

typedef struct _LINKED_LIST_S
{
	LIST_MEMBERS(struct _LINKED_LIST_S);
} LINKED_LIST, *PLINKED_LIST;

VOID *ListInitiate(INT Size)
{
	VOID *Entry = (void *)malloc(Size);

	if (!Entry)
		return NULL;

	memset(Entry, 0, Size);

	return Entry;
}

VOID *ListInsertAtHead(VOID **List, VOID *Entry)
{
	((PLINKED_LIST)Entry)->Next = (PLINKED_LIST)*List;
	*List = Entry;
	return Entry;
}

VOID *ListInsertAtTail(VOID **List, VOID *Entry)
{
	PLINKED_LIST Tail;

	if (*List == NULL)
	{
		*List = Entry;
		return Entry;
	}

	Tail = *List;
	while (Tail->Next != NULL)
		Tail = Tail->Next;

	Tail->Next = (PLINKED_LIST)Entry;

	return Entry;
}

VOID *ListFindRemove(VOID **List, VOID *Entry)
{
	PLINKED_LIST l;

	if (!(*List))
		return NULL;

	if (*List == Entry)
	{
		*List = ((PLINKED_LIST)*List)->Next;
		l = ((PLINKED_LIST)Entry)->Next;
		free(Entry);
		return Entry;
	}

	l = *List;
	while (l->Next != (PLINKED_LIST)Entry)
	{
		if (l->Next == NULL)
			return NULL;
		l = l->Next;
	}

	l->Next = ((PLINKED_LIST)Entry)->Next;
	l = l->Next;
	free(Entry);

	return l;
}