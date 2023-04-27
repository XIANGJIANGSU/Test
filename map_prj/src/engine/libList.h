#ifndef __LIB_LIST_H__
#define __LIB_LIST_H__

#ifdef __cplusplus
extern "C"{
#endif

/*����ͷ��㶨��*/
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

/*��ʼ��˫������*/
#define LIST_INIT(head) do{\
	(head)->pNext=(head)->pPriv=(head); \
	}while(0)

/*��ָ��Ԫ�أ�towhere�������item*/
#define LIST_ADD(item, towhere) do{\
	(item)->pNext = (towhere)->pNext;	\
	(item)->pPriv = (towhere);	\
	(item)->pPriv->pNext = (item);	\
	(item)->pNext->pPriv = (item);	\
	}while(0)

/*��ָ��Ԫ��ǰ����item*/
#define LIST_ADD_BEFORE(item, towhere) do {\
	LIST_ADD((towhere)->pPriv, item);	\
	} while (0)

/*ɾ��ĳ��Ԫ��*/
#define LIST_REMOVE(item) do{\
	(item)->pPriv->pNext = (item)->pNext;	\
	(item)->pNext->pPriv = (item)->pPriv;	\
	}while(0)

/*������������е�����Ԫ��*/
#define LIST_FOR_EACH_ITEM(item, head) \
	for((item)=(head)->pNext;(item)!=(head);(item)=(item)->pNext)

/*�����������*/
#define LIST_FOR_EACH_REV(item, head) \
	for((item)=(head)->pPriv;(item)!=(head);(item)=(item)->pPriv)

/*���ݱ��ڵ�(item)��ȡ������ڽṹ��ָ��*/
#define ASDE_LIST_ENTRY(item, type, member) \
	(type*)((char*)item-(char*)(&(((type*)0)->member)))

/*�ж������Ƿ�Ϊ��*/
#define LIST_IS_EMPTY(head) \
	((head)->pNext == (head))

/*��ȡָ��λ����һԪ��*/
#define LIST_PRIV_ITEM(item) \
	((item)->pPriv)

#ifdef __cplusplus
}
#endif

#endif