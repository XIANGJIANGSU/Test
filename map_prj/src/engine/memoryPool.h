/* 
 *  memoryPool.h (fixed memory pool) 
 *  内存池结构：
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
 *  内存池分为固定内存和可变内存
 *  可变内存是在程序中申请任意大小的内存（只能索引0~4M-4K的空间,即最大只能加载4M-4K空间的数据），其生命周期短，在程序运行期间就会被释放，造成内存碎片的原因/
 *  内存池索引包括（index：0~1022）
 *  固定内存是程序中申请的内存，直到程序结束后才释放，该内存大小不限，用链表管理，索引号为index = 1023
 */  
#ifndef _MEMORYPOOL_H_
#define _MEMORYPOOL_H_

#include "../define/mbaseType.h"

#define	MEMORY_STYLE_FIXED_USE		0		// 固定内存，在程序使用中内存一旦申请，直到程序结束才释放
#define	MEMORY_STYLE_ALTERABLE_USE	1		// 可变内存，在程序使用中申请大小不确定，且生命周期短，容易产生内存碎片，需要内存池进行管理

/*
 *	内存池索引大小
 *	例如pool_size = 1024，则可变内存可寻址的最大空间的大小为4K * 1022；索引0的内存池大小为1K，索引1023的内存池用于存放
 */ 
#define POOL_INDEX_SIZE_DEFAULT  1024//1536 //1024

/*
 *	计算最接近size的boundary的整数倍的整数
 */
#define SIZE_ALIGN(size, boundary) (((size) + ((boundary) - 1)) & ~((boundary) - 1))

typedef struct tagMemory_Unit_T*	LPMemory_Unit_T;
typedef struct  tagMemory_Unit_T
{
	LPMemory_Unit_T			m_pNext;			// 下一个可用空间
	f_uint32_t              m_nIndex;           //内存池集合中的索引
	f_uint32_t              m_nStyle;           //申请内存类型
	f_uint32_t              m_nSize;            //申请内存大小
	f_char_t*				m_pData;			//数据
} Memory_Unit_T;

typedef struct tagMemory_Pool_T
{
	f_uint16_t		  m_nUnitNum;				// 可变内存：内存池的连接的空闲单元个数 固定内存：内存池中已申请的单元个数
	LPMemory_Unit_T	  m_pIdleUnit;				// 空闲内存单元链表头
} Memory_Pool_T, *LPMemory_POOL_T;

//	基本设置操作
/*
 *	初始化内存池集合
 */
extern BOOL	InitMemoryPools(f_uint16_t  pool_num);
/*
 *	初始化默认大小内存池集合
 */
extern BOOL	InitMemoryPoolsDefault();
/*
 *	销毁内存池集合
 */
extern void	DestroyMemoryPools();

/*
 *	动态申请固定内存
 *  size: 申请内存的大小，单位为字节（B）
 */
extern	void*	NewFixedMemory(f_uint32_t size);
/*
 *	动态申请可变内存
 *  size: 申请内存的大小，单位为字节（B）
 */
extern	void*	NewAlterableMemory(f_uint32_t size);
/*
 *	动态申请内存
 *  size: 申请内存的大小，单位为字节（B）
 */
extern	void*	NewMemory(f_uint32_t size, f_uint8_t style);	
/*
 *	释放申请的内存
 */
extern  void	DeleteMemory(void* ptr);
/*
 *	释放申请的可变内存
 */
extern  void	DeleteAlterableMemory(void* ptr);


/*
 *	打印内存池集合信息
 */
extern void PrintMemoryPools();

#endif