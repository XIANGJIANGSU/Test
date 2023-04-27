#ifndef _mapNodeGen_h_ 
#define _mapNodeGen_h_ 

#include "../define/mbaseType.h"
#include "mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define VERTEX_USE_FLOAT
#undef VERTEX_USE_FLOAT

/*生成纹理坐标*/
void generateTexBuffer(void);
/* 生成三角形条带化后顶点的索引 */
void generateTileIndices(void);
/*生成鹰眼模式纹理坐标*/
void generateTexBufferEyeBird(void);
/* 生成鹰眼模式三角形条带化后顶点的索引 */
void generateTileIndicesEyeBird(void);

/* 功能：生成球面模型的网格数据(正常模式) */
f_int32_t qtmapNodeGenGlobalGrid(sQTMAPNODE *pNode, f_int16_t *terrain);
/* 功能：生成球面模型的网格数据(鹰眼模式) */
f_int32_t qtmapNodeGenGlobalGridForEyeBird(sQTMAPNODE *pNode);

/*功能：生成SVS阴影所需要的平面顶点网格数据*/
f_int32_t qtmapNodeGenGlobalGridFlat(sQTMAPNODE *pNode, f_int16_t *terrain);


f_uint64_t qtmapnodeGetStatus(sQTMAPNODE *pNode, f_int32_t line);
f_uint64_t qtmapnodeSetStatus(sQTMAPNODE * pNode, f_uint64_t status, f_int32_t line);

/*生成一个新的结点*/
//f_int32_t qtmapnodeCreate(sMAPHANDLE *pHandle, f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
//                          sQTMAPSUBTREE * pSubTree, sQTMAPNODE **ppNode);
f_int32_t qtmapnodeCreate(sMAPHANDLE *pHandle, f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                          sQTMAPSUBTREE * pSubTree, sQTMAPNODE **ppNode, BOOL emptyOrload);

f_int32_t qtmapNodeCreateList(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode);
f_int32_t qtmapNodeCreateListOverlook(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode);
f_int32_t qtmapNodeCreateEdgeList(sQTMAPNODE * pNode);
f_int32_t qtmapNodeCreateEdgeListOverlook(sQTMAPNODE * pNode);
/*创建vecShade模式下各个视角下的显示列表*/
f_int32_t qtmapNodeCreateVecShadeList(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode);


f_int32_t qtmapNodeCreateVboEs(sQTMAPNODE * pNode);
f_int32_t genConstVBOs(unsigned int* pibo, unsigned int* ptvbo, unsigned int* ptvbo_overlook, unsigned int* pibo_overlook);

f_int32_t qtmapNodeCreateSvsVboEs(sQTMAPNODE * pNode);
f_int32_t genSvsConstVBOs(unsigned int* pibo);

f_int32_t genVecShadeConstVBOs(unsigned int* pibo, unsigned int* ptvbo);

#ifdef _JM7200_
/*创建SVS方里网模式下各个视角下的显示列表*/
f_int32_t qtmapNodeCreateSVSGridList(sQTMAPNODE * pNode);
/*创建SVS地形告警模式下各个视角下的显示列表*/
f_int32_t qtmapNodeCreateSVSList(sQTMAPNODE * pNode);
#endif


/*返回内部链表中截止目前的节点数峰值，为调试用*/
f_int32_t qtmapGetNodePeakValue(VOIDPtr map_handle);
/* 返回当前缓冲区数 */
f_int32_t qtmapGetBufferCount(VOIDPtr map_handle);

f_int32_t qtmapNodeGenPartGrid(sQTMAPNODE *pNode);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
