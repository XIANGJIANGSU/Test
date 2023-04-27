#ifndef __LIB_LIST_H__
#define __LIB_LIST_H__

#ifdef __cplusplus
extern "C"{
#endif

/*链表头结点定义*/
typedef struct tagsList_Head
{
	struct tagsList_Head* pNext, *pPriv;
}stList_Head;

#define OFFSETOF(type, member) ((size_t) & ((type *)0)->member)

#define LIST_HEAD_INIT(name) {&(name), &(name)}

#define LIST_HEAD(name) \
	/*struct */stList_Head name = LIST_HEAD_INIT(name)

static void INIT_LIST_HEAD(stList_Head* list)
{
	list->pNext = list;
	list->pPriv = list;
}


// #define CONTAINER_OF(ptr, type, member) ({ \
// 	const typeof(((type*)0)->member) *__mptr = (ptr); \ 
// 	(type *)((char*)__mptr - OFFSETOF(type, member));})

/*初始化双向链表*/
#define LIST_INIT(head) do{\
	(head)->pNext=(head)->pPriv=(head); \
	}while(0)

/*在指定元素（towhere）后插入item*/
#define LIST_ADD(item, towhere) do{\
	(item)->pNext = (towhere)->pNext;	\
	(item)->pPriv = (towhere);	\
	(item)->pPriv->pNext = (item);	\
	(item)->pNext->pPriv = (item);	\
	}while(0)

/*在指定元素前插入item*/
#define LIST_ADD_BEFORE(item, towhere) do {\
	LIST_ADD((towhere)->pPriv, item);	\
	} while (0)

/*删除某个元素*/
#define LIST_REMOVE(item) do{\
	(item)->pPriv->pNext = (item)->pNext;	\
	(item)->pNext->pPriv = (item)->pPriv;	\
	}while(0)

/*正向遍历链表中的所有元素*/
#define LIST_FOR_EACH_ITEM(item, head) \
	for((item)=(head)->pNext;(item)!=(head);(item)=(item)->pNext)

/*反向遍历链表*/
#define LIST_FOR_EACH_REV(item, head) \
	for((item)=(head)->pPriv;(item)!=(head);(item)=(item)->pPriv)

/*根据本节点(item)获取结点所在结构体指针*/
#define ASDE_LIST_ENTRY(item, type, member) \
	(type*)((char*)item-(char*)(&(((type*)0)->member)))

/*判断链表是否为空*/
#define LIST_IS_EMPTY(head) \
	((head)->pNext == (head))

/*获取指定位置上一元素*/
#define LIST_PRIV_ITEM(item) \
	((item)->pPriv)

#ifdef __cplusplus
}
#endif

#endif