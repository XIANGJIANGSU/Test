#ifndef SEARCHDIR_H
#define SEARCHDIR_H



/*-----------------------------------------------------------
** 类型定义
**-----------------------------------------------------------
*/
/*链表结构*/
typedef struct chain_node
{
	char   path[128];           /*路径*/
	char   pathALL[256];	/*绝对路径，不包括文件名*/
	struct chain_node * next;   /*下一个节点指针*/
}ChainNode;

typedef struct head_node
{
	int number;        /*数目*/
	ChainNode * first; /*第一个节点指针*/
}ChainHead;


int search_dir(char * token, char * dir_path, ChainHead **head);


#endif