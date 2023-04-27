/* 
 *  memoryPool.h (fixed memory pool) 
 *  �ڴ�ؽṹ��
 *  ------------------------------------------------------------
 *  |index:0|index:1|index:2|index:3 |...|index:1022|index:1023|
 *  |size:1K|size:4K|size:8K|size:12K|...|size:3.99M|size:Large|
 *  ------------------------------------------------------------
 *     |		|				|						   |
 *  ------   ------			  ------			    ----------------
 *  | 1K |   | 4K |			  |12K |			    |fixed memmory0|
 *  ------   ------			  ------			    ----------------
 *    |													   |
 *  ------										    ----------------														
 *  | 1K |										    |fixed memmory1|
 *  ------										    ----------------
 *  �ڴ�ط�Ϊ�̶��ڴ�Ϳɱ��ڴ�
 *  �ɱ��ڴ����ڳ��������������С���ڴ棨ֻ������0~4M-4K�Ŀռ�,�����ֻ�ܼ���4M-4K�ռ�����ݣ������������ڶ̣��ڳ��������ڼ�ͻᱻ�ͷţ�����ڴ���Ƭ��ԭ��/
 *  �ڴ������������index��0~1022��
 *  �̶��ڴ��ǳ�����������ڴ棬ֱ�������������ͷţ����ڴ��С���ޣ����������������Ϊindex = 1023
 */  
#ifndef _MEMORYPOOL_H_
#define _MEMORYPOOL_H_

#include "../define/mbaseType.h"

#define	MEMORY_STYLE_FIXED_USE		0		// �̶��ڴ棬�ڳ���ʹ�����ڴ�һ�����룬ֱ������������ͷ�
#define	MEMORY_STYLE_ALTERABLE_USE	1		// �ɱ��ڴ棬�ڳ���ʹ���������С��ȷ�������������ڶ̣����ײ����ڴ���Ƭ����Ҫ�ڴ�ؽ��й���

/*
 *	�ڴ��������С
 *	����pool_size = 1024����ɱ��ڴ��Ѱַ�����ռ�Ĵ�СΪ4K * 1022������0���ڴ�ش�СΪ1K������1023���ڴ�����ڴ��
 */ 
#define POOL_INDEX_SIZE_DEFAULT  1024//1536 //1024

/*
 *	������ӽ�size��boundary��������������
 */
#define SIZE_ALIGN(size, boundary) (((size) + ((boundary) - 1)) & ~((boundary) - 1))

typedef struct tagMemory_Unit_T*	LPMemory_Unit_T;
typedef struct  tagMemory_Unit_T
{
	LPMemory_Unit_T			m_pNext;			// ��һ�����ÿռ�
	f_uint32_t              m_nIndex;           //�ڴ�ؼ����е�����
	f_uint32_t              m_nStyle;           //�����ڴ�����
	f_uint32_t              m_nSize;            //�����ڴ��С
	f_char_t*				m_pData;			//����
} Memory_Unit_T;

typedef struct tagMemory_Pool_T
{
	f_uint16_t		  m_nUnitNum;				// �ɱ��ڴ棺�ڴ�ص����ӵĿ��е�Ԫ���� �̶��ڴ棺�ڴ����������ĵ�Ԫ����
	LPMemory_Unit_T	  m_pIdleUnit;				// �����ڴ浥Ԫ����ͷ
} Memory_Pool_T, *LPMemory_POOL_T;

//	�������ò���
/*
 *	��ʼ���ڴ�ؼ���
 */
extern BOOL	InitMemoryPools(f_uint16_t  pool_num);
/*
 *	��ʼ��Ĭ�ϴ�С�ڴ�ؼ���
 */
extern BOOL	InitMemoryPoolsDefault();
/*
 *	�����ڴ�ؼ���
 */
extern void	DestroyMemoryPools();

/*
 *	��̬����̶��ڴ�
 *  size: �����ڴ�Ĵ�С����λΪ�ֽڣ�B��
 */
extern	void*	NewFixedMemory(f_uint32_t size);
/*
 *	��̬����ɱ��ڴ�
 *  size: �����ڴ�Ĵ�С����λΪ�ֽڣ�B��
 */
extern	void*	NewAlterableMemory(f_uint32_t size);
/*
 *	��̬�����ڴ�
 *  size: �����ڴ�Ĵ�С����λΪ�ֽڣ�B��
 */
extern	void*	NewMemory(f_uint32_t size, f_uint8_t style);	
/*
 *	�ͷ�������ڴ�
 */
extern  void	DeleteMemory(void* ptr);
/*
 *	�ͷ�����Ŀɱ��ڴ�
 */
extern  void	DeleteAlterableMemory(void* ptr);


/*
 *	��ӡ�ڴ�ؼ�����Ϣ
 */
extern void PrintMemoryPools();

#endif