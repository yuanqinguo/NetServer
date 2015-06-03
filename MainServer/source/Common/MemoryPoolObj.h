#ifndef MEMORYPOOL_OBJ_H
#define MEMORYPOOL_OBJ_H

#include "MemoryPool.h"
#include "locker.h"

class MemPoolObj
{
public:
	static MemoryPool* GetMemoryPool();

protected:
private:
	MemPoolObj(){};
	static MemoryPool m_mempool;
};

#endif