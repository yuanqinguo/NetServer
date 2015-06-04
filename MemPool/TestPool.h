#ifndef TESTPOOL_H
#define TESTPOOL_H
#include "MemoryPool.h"
/**
 * 实现利用内存池创建对象
 * 要求对象具有缺省构造函数
 */
template<typename T>
class ObjectManager
{
public:
	typedef T ObjectType;

	static ObjectType* Create(MemoryPool* pool)
	{
		void* pobject=pool->Malloc(sizeof(T));
		new(pobject) ObjectType();
		return static_cast<ObjectType*>(pobject);
	}
	static void Delete(MemoryPool* pool, ObjectType* pobject)
	{
		pobject->~ObjectType();
		pool->Free(pobject);
	}
};
#endif