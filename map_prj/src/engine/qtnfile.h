#ifndef __QTNFILE_H
#define __QTNFILE_H

typedef void * QTNFILE;
#include "../define/mbaseType.h"
#include "../mapApi/common.h"
 
#ifdef __cplusplus
extern "C" {
#endif

/*
	打开一个QTN文件，
	参数：cstrFileName -- 文件名称
	      pQTNFile -- 如果成功则返回文件句柄供后续调用使用
	返回值：
	     0 -- 成功，文件句柄在pQTNFile中
		-1 -- 文件打开失败
		-2 -- 文件格式错误
		-3 -- 内存分配失败
*/
f_int32_t qtnfileOpen(const f_char_t * cstrFileName, VOIDPtr * pQTNFile);

/*
	关闭一个已经打开的QTN文件，释放文件占用的资源
	参数：qtnfile -- 用qtnfileOpen返回的QTN文件句柄
	返回值：
	     0 -- 成功
		-1 -- 文件句柄无效
*/
f_int32_t qtnfileClose(VOIDPtr qtnfile);

/*
	得到一个已经打开的QTN文件中树的层数
	参数：qtnfile -- 用qtnfileOpen返回的QTN文件句柄
	返回值：
	    >0 -- 树的层数
		-1 -- 文件句柄无效
*/
f_int32_t qtnfileGetLayerCount(VOIDPtr qtnfile);

/*
	得到一个QTN文件中的基本图块数据
	参数：qtnfile -- 用qtnfileOpen返回的QTN文件句柄
	      layer -- 指定的层，从0开始
		  xidx, yidx -- 基本图块在层中的编号
		  pData -- 返回的数据
	返回值：
	     1 -- 数据正在调入
	     0 -- 成功
		-1 -- 文件句柄无效
		-2 -- 层号无效
		-3 -- xidx或yidx无效
		-4 -- 内存分配错误
		-5 -- 数据解码错误
*/
f_int32_t qtnfileGetData(VOIDPtr qtnfile, f_int32_t layer, f_int32_t xidx, f_int32_t yidx, void ** pData);

/*
layer:显示的层号
*/
void qtnSetLayerFunc(VOIDPtr map_handle, f_int32_t layer);

void qtnAddFile(const f_char_t * pFileName, f_int32_t rootlevel, f_int32_t rootxidx, f_int32_t rootyidx);

void qtnInit(void);
void qtnDeInit(void);


#ifdef __cplusplus
}
#endif

#endif /* __QTNFILE_H */