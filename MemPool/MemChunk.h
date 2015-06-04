#ifndef MEMORY_CHUNK_H
#define MEMORY_CHUNK_H
#include <cstdio>
#include <cassert>
#include <cstdlib>

#ifdef WIN32
#include <windows.h>
typedef CRITICAL_SECTION MUTEXTYPE;
#define INITMUTEX(hMutex) InitializeCriticalSection(&hMutex)
#define DELMUTEX(hMutex) DeleteCriticalSection(&hMutex)
#define LOCK(hMutex) EnterCriticalSection(&hMutex)
#define UNLOCK(hMutex) LeaveCriticalSection(&hMutex)
#else
#include <pthread.h>
typedef pthread_mutex_t MUTEXTYPE;
#define INITMUTEX(hMutex) pthread_mutex_init(&hMutex,NULL)
#define DELMUTEX(hMutex) pthread_mutex_destroy(&hMutex)
#define LOCK(hMutex) pthread_mutex_lock(&hMutex)
#define UNLOCK(hMutex) pthread_mutex_unlock(&hMutex)
#endif

class MemoryChunk;

/** @struct MemoryBlock
*
*/
struct BlockHeader
{
	MemoryChunk* pChunk;
	size_t len;
};
struct MemoryBlock;
struct BlockData
{
	union{
		MemoryBlock* pNext;
		char pBuffer;
	};
};
struct MemoryBlock
{
	BlockHeader header;
	BlockData data;
};

/** @class MemoryChunk
*
*/

class MemoryChunk
{
public:
	MemoryChunk(size_t size, int count)
	{
		INITMUTEX(hMutex);
		this->pFreeList=NULL;
		this->size=size;
		this->count=0;
		MemoryBlock* pBlock;
		while(count--)
		{
			pBlock=CreateBlock();
			if(!pBlock)
				break;
			pBlock->data.pNext=pFreeList;
			pFreeList=pBlock;
		}
	}

	~MemoryChunk()
	{
		int tempcount=0;
		MemoryBlock* pBlock;
		while(pBlock=pFreeList)
		{
			pFreeList=pBlock->data.pNext;
			DeleteBlock(pBlock);
			++tempcount;
		}
		assert(tempcount==count);//!确保释放完全
		DELMUTEX(hMutex);
	}

	void* malloc()
	{
		MemoryBlock* pBlock;
		LOCK(hMutex);
		if(pFreeList)
		{
			pBlock=pFreeList;
			pFreeList=pBlock->data.pNext;
		}
		else
		{
			if(!(pBlock=CreateBlock()))
			{
				UNLOCK(hMutex);
				return NULL;
			}
		}
		UNLOCK(hMutex);
		return &pBlock->data.pBuffer;
	}

	static void free(void* pMem)
	{
		MemoryBlock* pBlock=(MemoryBlock*)((char*)pMem-sizeof(BlockHeader));
		pBlock->header.pChunk->free(pBlock);
	}

	void free(MemoryBlock* pBlock)
	{
		LOCK(hMutex);
		pBlock->data.pNext=pFreeList;
		pFreeList=pBlock;
		UNLOCK(hMutex);
	}

	MemoryChunk* Next(){ return pNext; }

protected:
	MemoryBlock* CreateBlock()
	{
		MemoryBlock* pBlock=(MemoryBlock*)::malloc(sizeof(BlockHeader)+size);

		if(pBlock)
		{

			pBlock->header.pChunk=this;
			pBlock->header.len=size;

			++count;
		}
		return pBlock;
	}

	void DeleteBlock(MemoryBlock* pBlock)
	{
		::free(pBlock);
	}
private:
	MemoryBlock* pFreeList;
	size_t size;//!Block大小
	int count;//!Block数目
	MemoryChunk* pNext;
	MUTEXTYPE hMutex;
};
#endif