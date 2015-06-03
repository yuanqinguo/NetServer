#include "MemoryPoolObj.h"

MemoryPool MemPoolObj::m_mempool;


MemoryPool* MemPoolObj::GetMemoryPool()
{
	return &m_mempool;
}