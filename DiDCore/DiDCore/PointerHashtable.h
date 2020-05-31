// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once 

#include "stdafx.h"

//#define POINTER_HASHTABLE_BUCKET_COUNT	1399
//#define POINTER_HASHTABLE_BUCKET_COUNT	20011
#define POINTER_HASHTABLE_BUCKET_COUNT		300007

template<class T, int BucketSize = POINTER_HASHTABLE_BUCKET_COUNT>
class CPointerHashtable {
	friend class CHeapMonitor;
public:

	CPointerHashtable()
	{
		allocator = NULL;
		deallocator = NULL;
		memset(Buckets, 0, sizeof(Buckets));

		for(size_t i=0; i<BucketSize; i++)
		{
			InitializeCriticalSection(&Locks[i]);
		}
	}

	~CPointerHashtable()
	{
		// todo: free all list entries
		// HeapDestroy handles this requirement in CHeapMonitor
	}

	LPVOID (*allocator)(size_t cb);
	void (*deallocator)(LPVOID lpv);
	
	T *GetEntry(LPVOID lpv, BOOL bLock = TRUE)
	{
		T *entry = NULL;
		DWORD index = HashPointer(lpv);

		if(bLock)
			LockBucket(index);

		ListEntry *ple = Buckets[index].Entries;
		
		while(ple)
		{
			if(ple->Idx == lpv)
			{
				entry = &(ple->Entry);
				break;
			}

			ple = ple->Next;
		}

		if(bLock)
			UnlockBucket(index);

		return entry;
	}

	void SetEntry(LPVOID lpv, T *ent, BOOL bLock = TRUE)
	{
		T *entry = NULL;
		DWORD index = HashPointer(lpv);

		if(bLock)
			LockBucket(index);

		ListEntry *ple = Buckets[index].Entries, *last = NULL;
		
		while(ple)
		{
			if(ple->Idx == lpv)
			{
				entry = &(ple->Entry);
				break;
			}

			last = ple;
			ple = ple->Next;
		}

		if(!entry)
		{
			if(allocator)
				ple = reinterpret_cast<ListEntry *>(allocator(sizeof(ListEntry)));
			else
				ple = reinterpret_cast<ListEntry *>(malloc(sizeof(ListEntry)));

			if(!ple)
			{
				if(!allocator)
				{
					if(bLock)
						UnlockBucket(index);
					
					throw new std::exception("Allocation failure in CPointerHashtable");
				}
				else
					goto Cleanup; // could new(ptr) std::exception I guess
			}

			ple->Next = NULL;
			ple->Idx = lpv;
			entry = &(ple->Entry);

			if(last)
				last->Next = ple;
			else
				Buckets[index].Entries = ple;
		}

		*entry = *ent;
Cleanup:
		if(bLock)
			UnlockBucket(index);
	}

	void RemoveEntry(LPVOID lpv, BOOL bLock = TRUE)
	{
		DWORD index = HashPointer(lpv);
		
		if(bLock)
			LockBucket(index);
		
		ListEntry *ple = Buckets[index].Entries, *last = NULL;
		
		while(ple)
		{
			if(ple->Idx == lpv)
				break;
			
			last = ple;
			ple = ple->Next;
		}

		if(!ple)
			goto Cleanup;

		if(last)
			last->Next = ple->Next;
		else
			Buckets[index].Entries = NULL;

		if(deallocator)
			deallocator(ple);
		else
			free(ple);

Cleanup:
		if(bLock)
			UnlockBucket(index);
	}

	void LockBucket(DWORD dwBucketIndex)
	{
		EnterCriticalSection(&Locks[dwBucketIndex]);
	}

	void UnlockBucket(DWORD dwBucketIndex)
	{
		LeaveCriticalSection(&Locks[dwBucketIndex]);
	}

	typedef struct _ListEntry {
		struct _ListEntry *Next;
		LPVOID Idx;
		T Entry;
	} ListEntry;

private:

	DWORD HashPointer(LPVOID lpv)
	{
		DWORD dwpv = (DWORD)lpv, dwHashCode = 0;
		
		// fairly good hash function
		dwHashCode = Utils::RSHash((char *)&lpv, sizeof(LPVOID));
		dwHashCode %= BucketSize;
		
		return dwHashCode;
	}

	struct {
		ListEntry *Entries;
	} Buckets[BucketSize];

	CRITICAL_SECTION Locks[BucketSize];
};