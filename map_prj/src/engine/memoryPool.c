#include "memoryPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osAdapter.h"

#define ThrowMessage printf

static LPMemory_POOL_T g_MemoryPools = NULL;		// 内存池集合
static f_uint32_t	g_MaxUnitSize = 0;				// 内存池最大可以申请到的空间
static f_uint16_t	g_PoolNum = 0;
static SEMID 		g_MemorySet;					//内存池互斥信号量
static f_uint32_t*  g_PoolCreateUnitNum;                //每个内存池已申请的内存单元个数
/*
 *	初始化内存池集合
 */
BOOL InitMemoryPools(f_uint16_t  pool_num)
{
	// 内存池集合所需要的空间大小
	f_int32_t byte_num = sizeof(Memory_Pool_T) * pool_num;
	if(pool_num < 3)
	{
		return FALSE;
	}
	if((g_MemoryPools = (LPMemory_POOL_T)malloc(byte_num)) == NULL)
	{
		return FALSE;
	}
	// 初始化内存池集合空间
	memset(g_MemoryPools, 0, byte_num);

	if((g_PoolCreateUnitNum = (f_uint32_t*)malloc(pool_num * sizeof(f_uint32_t))) == NULL)
	{
		return FALSE;
	}
	// 初始化内存池已申请的内存单元个数
	memset(g_PoolCreateUnitNum, 0, pool_num * sizeof(f_uint32_t));

	// 计算内存池最大可以申请到的可变内存空间,因为规定索引0存储1K空间内存池的索引，索引（pool_num - 1）存储固定内存池，/
	// 所以最大可变内存空间为
	g_MaxUnitSize = (pool_num - 2) << 12;	// (pool_num - 2) * 4 * 1024
	g_PoolNum = pool_num;

	g_MemorySet = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(g_MemorySet);	

	return TRUE;
}

/*
 *	初始化默认大小内存池集合
 */
BOOL InitMemoryPoolsDefault()
{
	return (InitMemoryPools(POOL_INDEX_SIZE_DEFAULT));
}

/*
 *	销毁内存池集合
 */
void DestroyMemoryPools()
{
	LPMemory_Unit_T t_pUnit = NULL;
	f_char_t* t_pFirst = NULL;
	f_uint16_t i = 0;
	// 分别销毁内存池中的空间，返还给操作系统
	for (i = 0; i < g_PoolNum; ++i)
	{
		while (g_MemoryPools[i].m_pIdleUnit != NULL)
		{
			t_pUnit = g_MemoryPools[i].m_pIdleUnit;
			g_MemoryPools[i].m_pIdleUnit = t_pUnit->m_pNext;
			t_pFirst = (f_char_t*)t_pUnit;
			free(t_pFirst);
		}
	}

	// 销毁存储内存池已申请的内存单元个数的空间
	free(g_PoolCreateUnitNum);

	// 销毁内存池
	free(g_MemoryPools);

}


/*
 *	打印内存池集合信息
 */
void PrintMemoryPools()
{
	LPMemory_Unit_T t_pUnit = NULL;
	f_uint16_t i = 0;
	f_uint16_t unitSize = 0;
	f_uint32_t totalFixedMemorySize = 0;

	for (i = 0; i < g_PoolNum; i++)
	{
		if (0 == i)
		{
			unitSize = 1;
		}else{
			unitSize = 4*i;
		}

		if (i != (g_PoolNum - 1))
		{
			if (g_PoolCreateUnitNum[i] > 0)
			{
				printf("memoryPoolIndex=%d memoryStyle=%d unitSize=%dKB createUnitNum=%d idleUnitNum=%d\n",
					i, MEMORY_STYLE_ALTERABLE_USE, unitSize, g_PoolCreateUnitNum[i], g_MemoryPools[i].m_nUnitNum);
			}
		}else{
			t_pUnit = g_MemoryPools[i].m_pIdleUnit;
			while (t_pUnit != NULL)
			{
				totalFixedMemorySize += t_pUnit->m_nSize;
				t_pUnit = t_pUnit->m_pNext;
			}
			t_pUnit = NULL;
			printf("memoryPoolIndex=%d memoryStyle=%d createUnitNum=%d unitSize=%dB\n",
				i, MEMORY_STYLE_FIXED_USE, g_PoolCreateUnitNum[i],totalFixedMemorySize);
		}
	}

}

/*
 *	动态申请固定内存
 *  size: 申请内存的大小，单位为字节（B）
 */
void*	NewFixedMemory(f_uint32_t size)
{
	return (NewMemory(size, MEMORY_STYLE_FIXED_USE));
}

/*
 *	动态申请可变内存
 *  size: 申请内存的大小，单位为字节（B）
 */
void*	NewAlterableMemory(f_uint32_t size)
{
    return (NewMemory(size, MEMORY_STYLE_ALTERABLE_USE));
}

/*
 *	动态申请内存
 *  size: 申请内存的大小，单位为字节（B）
 */
void* NewMemory(f_uint32_t size, f_uint8_t style)
{
	LPMemory_Unit_T t_pUint = NULL; 
	f_char_t* t_pMemory = NULL;
	f_uint32_t t_Size = 0;						// 需要申请的内存大小
	f_uint16_t t_Index = 0;						// 该内存单元在内存池集合中的索引

	takeSem(g_MemorySet, FOREVER_WAIT);

	if (size <= 0)
	{
		return NULL;
	}
	if(style == MEMORY_STYLE_ALTERABLE_USE)		// 可变内存大小从0（B）~g_MaxUnitSize（B）
	{
		if (size <= 1024)						// 申请的空间小于1K
		{
			t_Size = 1024;
			t_Index = 0;
		}
		else
		{
			t_Size = SIZE_ALIGN(size, 4096);	// 实际需要分配的空间，4K空间的整数倍
			if(size >  g_MaxUnitSize)			// 申请可变空间应该在1K~g_MaxUnitSize范围内
			{
				ThrowMessage("动态申请内存%d出错，超出可变内存申请长度", "申请内存出错", size);
				giveSem(g_MemorySet);
				return NULL;
			}
			if (2097152 == t_Size)
			{
				printf("aaa");
			}
			t_Index = t_Size >> 12;				// t_Size / 4096B
		}
		if (g_MemoryPools[t_Index].m_pIdleUnit == NULL)		// 申请空间
		{
			t_pMemory = (f_char_t*)malloc(t_Size + sizeof(Memory_Unit_T));
			if(t_pMemory == NULL)
			{
				ThrowMessage("系统申请内存空间出错", "系统出错");
				giveSem(g_MemorySet);
				return NULL;
			}
			memset(t_pMemory, 0, t_Size + sizeof(Memory_Unit_T));			
			t_pUint = (LPMemory_Unit_T)t_pMemory;
			t_pUint->m_pData = t_pMemory + sizeof(Memory_Unit_T);
			t_pUint->m_nStyle = style;
			t_pUint->m_nSize = t_Size;
			t_pUint->m_pNext = NULL;
			t_pUint->m_nIndex = t_Index;
			g_PoolCreateUnitNum[t_Index]++;
		}
		else				// 利用已有的空间，并更改内存池索引
		{
			t_pUint = g_MemoryPools[t_Index].m_pIdleUnit;
			g_MemoryPools[t_Index].m_pIdleUnit = t_pUint->m_pNext;
			g_MemoryPools[t_Index].m_nUnitNum--;
			t_pUint->m_pNext = NULL;
		}
	}
	else if(style == MEMORY_STYLE_FIXED_USE)
	{
		t_Size = size;
		t_Index = g_PoolNum - 1;
		// 直接申请，并注册
		t_pMemory = (f_char_t*)malloc(t_Size + sizeof(Memory_Unit_T));
		if(t_pMemory == NULL)
		{
			ThrowMessage("系统申请内存空间出错", "系统出错");
			giveSem(g_MemorySet);
			return NULL;
		}
		memset(t_pMemory, 0, t_Size + sizeof(Memory_Unit_T));		
		t_pUint = (LPMemory_Unit_T)t_pMemory;
		t_pUint->m_pData = t_pMemory + sizeof(Memory_Unit_T);
		t_pUint->m_nStyle = style;
		t_pUint->m_nSize = t_Size;
		t_pUint->m_pNext = NULL;
		t_pUint->m_nIndex = t_Index;
		// 在内存池中注册
		t_pUint->m_pNext = g_MemoryPools[t_Index].m_pIdleUnit;
		g_MemoryPools[t_Index].m_pIdleUnit = t_pUint;
		g_MemoryPools[t_Index].m_nUnitNum++;
		g_PoolCreateUnitNum[t_Index]++;
	}
	else
	{
		ThrowMessage("动态申请内存出错，内存类型不正确", "申请内存出错");
		giveSem(g_MemorySet);
		return NULL;
	}

	giveSem(g_MemorySet);
	return (void*)(t_pUint->m_pData);
}

/*
 *	释放申请的内存
 *  需要把释放的内存空间返回给内存池，而不是OS
 */
void DeleteMemory(void* ptr)
{
	f_char_t* t_Ptr = NULL;
	LPMemory_Unit_T t_pUint = NULL;

	takeSem(g_MemorySet, FOREVER_WAIT);
	
	if(ptr == NULL)
	{
		giveSem(g_MemorySet);
		return;
	}
	// 移动指针到内存单元的开始
	t_Ptr = (f_char_t*)ptr;
	t_Ptr -= sizeof(Memory_Unit_T);			// 内存单元的首字节
	t_pUint = (LPMemory_Unit_T)t_Ptr;		// 内存单元LPMemory_Unit_T首节点
	// 把该内存挂接在内存池集合中
	if(g_MemoryPools == NULL)
	{
		ThrowMessage("内存池集合为空", "删除内存出错");
		giveSem(g_MemorySet);
		return;
	}
	switch(t_pUint->m_nStyle)
	{
	case MEMORY_STYLE_ALTERABLE_USE:		// 可变内存
		{
			// 找到在内存池集合的索引，进行挂接
			if (t_pUint->m_nIndex > (f_uint32_t)(g_PoolNum - 2))
			{
				ThrowMessage("未找到该内存在内存池集合中的索引号", "删除内存出错");
				giveSem(g_MemorySet);
				return;
			}

			// 为节省内存，每个内存池单元仅留下20个空闲单元，大于20时直接free
			if (g_MemoryPools[t_pUint->m_nIndex].m_nUnitNum > 20)
			{
				g_PoolCreateUnitNum[t_pUint->m_nIndex]--;
				free(t_pUint);
				giveSem(g_MemorySet);
				return;
			}

			// 重置数据内存
			if (t_pUint->m_nIndex == 0)
			{
				memset(ptr, 0, 1024);
			}
			else
			{
				memset(ptr, 0, t_pUint->m_nIndex << 12);		// memset(ptr, NULL, t_pUint->m_nIndex * 4096)
			}
			// 将该内存在对应索引的内存池进行注册
			t_pUint->m_pNext = g_MemoryPools[t_pUint->m_nIndex].m_pIdleUnit;
			g_MemoryPools[t_pUint->m_nIndex].m_pIdleUnit = t_pUint;
			g_MemoryPools[t_pUint->m_nIndex].m_nUnitNum++;
			break;
		}
	case MEMORY_STYLE_FIXED_USE:			// 固定内存
		{
			// 固定内存要放到程序退出后才释放
			ThrowMessage("固定内存要放到程序退出后才释放", "删除内存出错");
			giveSem(g_MemorySet);
			break;
		}
	default:
		{
			ThrowMessage("该内存是未识别类型", "删除内存出错");
			giveSem(g_MemorySet);
			return;
		}
	}

	giveSem(g_MemorySet);
	return;
}

/*
 *	释放申请的可变内存
 */
void DeleteAlterableMemory(void* ptr)
{
	DeleteMemory(ptr);
}
