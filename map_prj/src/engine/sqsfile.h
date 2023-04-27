#ifndef __SQSFILE_H
#define __SQSFILE_H

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	打开一个SQS文件，
	参数：cstrFileName -- 文件名称
	返回值：
	    文件句柄
*/
VOIDPtr sqsfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName);

/*
	关闭一个已经打开的SQS文件，释放文件占用的资源
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	     0 -- 成功
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileClose(VOIDPtr sqsfile);

/*
	得到一个已经打开的SQS文件中树的层数
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 树的层数
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileGetLevels(VOIDPtr sqsfile);

/*
	得到一个已经打开的SQS文件中树的起始层号
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 树的起始层号
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileGetLevelstart(VOIDPtr sqsfile);

/*
	得到一个已经打开的SQS文件中基本图块的大小
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 基本图块的大小
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileGetTileSize(VOIDPtr sqsfile);

/*
	得到一个已经打开的SQS文件中基本图块的MIPMAP层数
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 基本图块的MIPMAP层数
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileGetMipMapCount(VOIDPtr sqsfile);

/*
功能：判断节点是否在文件的范围内
输入：    
    level -- 指定的层，从0开始
	xidx, yidx -- 基本图块在层中的编号
	param  文件句柄所在的地址
输出：
    0  在
    -1 不在
*/
f_int32_t imgSqsIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param);

f_int32_t loadsqs(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg);
f_int32_t sqs2tex(f_uint64_t param , f_uint8_t *pImg);

#ifdef WIN32
void sqsTextureFunInit(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SQSFILE_H */
