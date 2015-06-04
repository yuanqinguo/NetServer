#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include "MemChunk.h"
/** @ MemoryPool.h
 * 定义实现内存池
 * 采用固定大小策略进行内存管理与分配
 * 减少因大量小内存分配导致的内存碎片增加
 */
struct HeapHeader
{
	size_t size;
};
struct MemoryHeap
{
	HeapHeader header;
	char pBuffer;
};

class MemoryPool
{
public:
	typedef enum{MAX_SIZE=1024,MIN_SIZE=sizeof(MemoryChunk*)};
	MemoryPool()
	{
		//计算分配块的个数
		chunkcount=0;
		for(size_t size=MIN_SIZE; size<=MAX_SIZE; size*=2)
			++chunkcount;

		pChunkList=new MemoryChunk*[chunkcount];

		//每块的内存大小，2的次方递增，4 8 16....
		int index=0;
		for(size_t size=MIN_SIZE; size<=MAX_SIZE; size*=2)
		{
			pChunkList[index++]=new MemoryChunk(size,1000);
		}
	}

	~MemoryPool()
	{
		for(int index=0; index<chunkcount; ++index)
		{
			delete pChunkList[index];
		}
		delete[] pChunkList;
	}

	void* Malloc(size_t size)
	{
		//若申请大于最大块，则额外申请内存
		if(size>MAX_SIZE)
		{
			return malloc(size);
		}
		//查找对应申请内存大小或大余申请内存大小的块
		int index=0;
		for(size_t tsize=MIN_SIZE; tsize<=MAX_SIZE; tsize*=2)
		{
			if(tsize>=size)break;
			++index;
		}
		//返回对应内存块
		return pChunkList[index]->malloc();
	}

	//释放内存块
	void Free(void* pMem)
	{
		if(!free(pMem))MemoryChunk::free(pMem);
	}
protected:
	void* malloc(size_t size)
	{
		//额外分配内存，并返回buffer地址，即为真正使用的内存块开始地址
		MemoryHeap* pHeap=(MemoryHeap*)::malloc(sizeof(HeapHeader)+size);
		if(pHeap)
		{
			pHeap->header.size=size;
			return &pHeap->pBuffer;
		}
		return NULL;
	}

	bool free(void* pMem)
	{
		//释放额外分配的内存地址，进行地址偏移，找到头部开始地址，将头部一同释放
		MemoryHeap* pHeap=(MemoryHeap*)((char*)pMem-sizeof(HeapHeader));
		if(pHeap->header.size>MAX_SIZE)
		{
			::free(pHeap);
			return true;
		}
		return false;
	}
private:
	MemoryChunk** pChunkList;//内存池块数组
	int chunkcount;//内存池块总数
};
#endif //MEMORYPOOL_H