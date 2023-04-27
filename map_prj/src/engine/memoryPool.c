#include "memoryPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osAdapter.h"

#define ThrowMessage printf

static LPMemory_POOL_T g_MemoryPools = NULL;		// �ڴ�ؼ���
static f_uint32_t	g_MaxUnitSize = 0;				// �ڴ�����������뵽�Ŀռ�
static f_uint16_t	g_PoolNum = 0;
static SEMID 		g_MemorySet;					//�ڴ�ػ����ź���
static f_uint32_t*  g_PoolCreateUnitNum;                //ÿ���ڴ����������ڴ浥Ԫ����
/*
 *	��ʼ���ڴ�ؼ���
 */
BOOL InitMemoryPools(f_uint16_t  pool_num)
{
	// �ڴ�ؼ�������Ҫ�Ŀռ��С
	f_int32_t byte_num = sizeof(Memory_Pool_T) * pool_num;
	if(pool_num < 3)
	{
		return FALSE;
	}
	if((g_MemoryPools = (LPMemory_POOL_T)malloc(byte_num)) == NULL)
	{
		return FALSE;
	}
	// ��ʼ���ڴ�ؼ��Ͽռ�
	memset(g_MemoryPools, 0, byte_num);

	if((g_PoolCreateUnitNum = (f_uint32_t*)malloc(pool_num * sizeof(f_uint32_t))) == NULL)
	{
		return FALSE;
	}
	// ��ʼ���ڴ����������ڴ浥Ԫ����
	memset(g_PoolCreateUnitNum, 0, pool_num * sizeof(f_uint32_t));

	// �����ڴ�����������뵽�Ŀɱ��ڴ�ռ�,��Ϊ�涨����0�洢1K�ռ��ڴ�ص�������������pool_num - 1���洢�̶��ڴ�أ�/
	// �������ɱ��ڴ�ռ�Ϊ
	g_MaxUnitSize = (pool_num - 2) << 12;	// (pool_num - 2) * 4 * 1024
	g_PoolNum = pool_num;

	g_MemorySet = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(g_MemorySet);	

	return TRUE;
}

/*
 *	��ʼ��Ĭ�ϴ�С�ڴ�ؼ���
 */
BOOL InitMemoryPoolsDefault()
{
	return (InitMemoryPools(POOL_INDEX_SIZE_DEFAULT));
}

/*
 *	�����ڴ�ؼ���
 */
void DestroyMemoryPools()
{
	LPMemory_Unit_T t_pUnit = NULL;
	f_char_t* t_pFirst = NULL;
	f_uint16_t i = 0;
	// �ֱ������ڴ���еĿռ䣬����������ϵͳ
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

	// ���ٴ洢�ڴ����������ڴ浥Ԫ�����Ŀռ�
	free(g_PoolCreateUnitNum);

	// �����ڴ��
	free(g_MemoryPools);

}


/*
 *	��ӡ�ڴ�ؼ�����Ϣ
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
 *	��̬����̶��ڴ�
 *  size: �����ڴ�Ĵ�С����λΪ�ֽڣ�B��
 */
void*	NewFixedMemory(f_uint32_t size)
{
	return (NewMemory(size, MEMORY_STYLE_FIXED_USE));
}

/*
 *	��̬����ɱ��ڴ�
 *  size: �����ڴ�Ĵ�С����λΪ�ֽڣ�B��
 */
void*	NewAlterableMemory(f_uint32_t size)
{
    return (NewMemory(size, MEMORY_STYLE_ALTERABLE_USE));
}

/*
 *	��̬�����ڴ�
 *  size: �����ڴ�Ĵ�С����λΪ�ֽڣ�B��
 */
void* NewMemory(f_uint32_t size, f_uint8_t style)
{
	LPMemory_Unit_T t_pUint = NULL; 
	f_char_t* t_pMemory = NULL;
	f_uint32_t t_Size = 0;						// ��Ҫ������ڴ��С
	f_uint16_t t_Index = 0;						// ���ڴ浥Ԫ���ڴ�ؼ����е�����

	takeSem(g_MemorySet, FOREVER_WAIT);

	if (size <= 0)
	{
		return NULL;
	}
	if(style == MEMORY_STYLE_ALTERABLE_USE)		// �ɱ��ڴ��С��0��B��~g_MaxUnitSize��B��
	{
		if (size <= 1024)						// ����Ŀռ�С��1K
		{
			t_Size = 1024;
			t_Index = 0;
		}
		else
		{
			t_Size = SIZE_ALIGN(size, 4096);	// ʵ����Ҫ����Ŀռ䣬4K�ռ��������
			if(size >  g_MaxUnitSize)			// ����ɱ�ռ�Ӧ����1K~g_MaxUnitSize��Χ��
			{
				ThrowMessage("��̬�����ڴ�%d���������ɱ��ڴ����볤��", "�����ڴ����", size);
				giveSem(g_MemorySet);
				return NULL;
			}
			if (2097152 == t_Size)
			{
				printf("aaa");
			}
			t_Index = t_Size >> 12;				// t_Size / 4096B
		}
		if (g_MemoryPools[t_Index].m_pIdleUnit == NULL)		// ����ռ�
		{
			t_pMemory = (f_char_t*)malloc(t_Size + sizeof(Memory_Unit_T));
			if(t_pMemory == NULL)
			{
				ThrowMessage("ϵͳ�����ڴ�ռ����", "ϵͳ����");
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
		else				// �������еĿռ䣬�������ڴ������
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
		// ֱ�����룬��ע��
		t_pMemory = (f_char_t*)malloc(t_Size + sizeof(Memory_Unit_T));
		if(t_pMemory == NULL)
		{
			ThrowMessage("ϵͳ�����ڴ�ռ����", "ϵͳ����");
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
		// ���ڴ����ע��
		t_pUint->m_pNext = g_MemoryPools[t_Index].m_pIdleUnit;
		g_MemoryPools[t_Index].m_pIdleUnit = t_pUint;
		g_MemoryPools[t_Index].m_nUnitNum++;
		g_PoolCreateUnitNum[t_Index]++;
	}
	else
	{
		ThrowMessage("��̬�����ڴ�����ڴ����Ͳ���ȷ", "�����ڴ����");
		giveSem(g_MemorySet);
		return NULL;
	}

	giveSem(g_MemorySet);
	return (void*)(t_pUint->m_pData);
}

/*
 *	�ͷ�������ڴ�
 *  ��Ҫ���ͷŵ��ڴ�ռ䷵�ظ��ڴ�أ�������OS
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
	// �ƶ�ָ�뵽�ڴ浥Ԫ�Ŀ�ʼ
	t_Ptr = (f_char_t*)ptr;
	t_Ptr -= sizeof(Memory_Unit_T);			// �ڴ浥Ԫ�����ֽ�
	t_pUint = (LPMemory_Unit_T)t_Ptr;		// �ڴ浥ԪLPMemory_Unit_T�׽ڵ�
	// �Ѹ��ڴ�ҽ����ڴ�ؼ�����
	if(g_MemoryPools == NULL)
	{
		ThrowMessage("�ڴ�ؼ���Ϊ��", "ɾ���ڴ����");
		giveSem(g_MemorySet);
		return;
	}
	switch(t_pUint->m_nStyle)
	{
	case MEMORY_STYLE_ALTERABLE_USE:		// �ɱ��ڴ�
		{
			// �ҵ����ڴ�ؼ��ϵ����������йҽ�
			if (t_pUint->m_nIndex > (f_uint32_t)(g_PoolNum - 2))
			{
				ThrowMessage("δ�ҵ����ڴ����ڴ�ؼ����е�������", "ɾ���ڴ����");
				giveSem(g_MemorySet);
				return;
			}

			// Ϊ��ʡ�ڴ棬ÿ���ڴ�ص�Ԫ������20�����е�Ԫ������20ʱֱ��free
			if (g_MemoryPools[t_pUint->m_nIndex].m_nUnitNum > 20)
			{
				g_PoolCreateUnitNum[t_pUint->m_nIndex]--;
				free(t_pUint);
				giveSem(g_MemorySet);
				return;
			}

			// ���������ڴ�
			if (t_pUint->m_nIndex == 0)
			{
				memset(ptr, 0, 1024);
			}
			else
			{
				memset(ptr, 0, t_pUint->m_nIndex << 12);		// memset(ptr, NULL, t_pUint->m_nIndex * 4096)
			}
			// �����ڴ��ڶ�Ӧ�������ڴ�ؽ���ע��
			t_pUint->m_pNext = g_MemoryPools[t_pUint->m_nIndex].m_pIdleUnit;
			g_MemoryPools[t_pUint->m_nIndex].m_pIdleUnit = t_pUint;
			g_MemoryPools[t_pUint->m_nIndex].m_nUnitNum++;
			break;
		}
	case MEMORY_STYLE_FIXED_USE:			// �̶��ڴ�
		{
			// �̶��ڴ�Ҫ�ŵ������˳�����ͷ�
			ThrowMessage("�̶��ڴ�Ҫ�ŵ������˳�����ͷ�", "ɾ���ڴ����");
			giveSem(g_MemorySet);
			break;
		}
	default:
		{
			ThrowMessage("���ڴ���δʶ������", "ɾ���ڴ����");
			giveSem(g_MemorySet);
			return;
		}
	}

	giveSem(g_MemorySet);
	return;
}

/*
 *	�ͷ�����Ŀɱ��ڴ�
 */
void DeleteAlterableMemory(void* ptr)
{
	DeleteMemory(ptr);
}
