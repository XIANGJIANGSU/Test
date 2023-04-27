#ifndef _mapNodeGen_h_ 
#define _mapNodeGen_h_ 

#include "../define/mbaseType.h"
#include "mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define VERTEX_USE_FLOAT
#undef VERTEX_USE_FLOAT

/*������������*/
void generateTexBuffer(void);
/* �����������������󶥵������ */
void generateTileIndices(void);
/*����ӥ��ģʽ��������*/
void generateTexBufferEyeBird(void);
/* ����ӥ��ģʽ�������������󶥵������ */
void generateTileIndicesEyeBird(void);

/* ���ܣ���������ģ�͵���������(����ģʽ) */
f_int32_t qtmapNodeGenGlobalGrid(sQTMAPNODE *pNode, f_int16_t *terrain);
/* ���ܣ���������ģ�͵���������(ӥ��ģʽ) */
f_int32_t qtmapNodeGenGlobalGridForEyeBird(sQTMAPNODE *pNode);

/*���ܣ�����SVS��Ӱ����Ҫ��ƽ�涥����������*/
f_int32_t qtmapNodeGenGlobalGridFlat(sQTMAPNODE *pNode, f_int16_t *terrain);


f_uint64_t qtmapnodeGetStatus(sQTMAPNODE *pNode, f_int32_t line);
f_uint64_t qtmapnodeSetStatus(sQTMAPNODE * pNode, f_uint64_t status, f_int32_t line);

/*����һ���µĽ��*/
//f_int32_t qtmapnodeCreate(sMAPHANDLE *pHandle, f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
//                          sQTMAPSUBTREE * pSubTree, sQTMAPNODE **ppNode);
f_int32_t qtmapnodeCreate(sMAPHANDLE *pHandle, f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                          sQTMAPSUBTREE * pSubTree, sQTMAPNODE **ppNode, BOOL emptyOrload);

f_int32_t qtmapNodeCreateList(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode);
f_int32_t qtmapNodeCreateListOverlook(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode);
f_int32_t qtmapNodeCreateEdgeList(sQTMAPNODE * pNode);
f_int32_t qtmapNodeCreateEdgeListOverlook(sQTMAPNODE * pNode);
/*����vecShadeģʽ�¸����ӽ��µ���ʾ�б�*/
f_int32_t qtmapNodeCreateVecShadeList(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode);


f_int32_t qtmapNodeCreateVboEs(sQTMAPNODE * pNode);
f_int32_t genConstVBOs(unsigned int* pibo, unsigned int* ptvbo, unsigned int* ptvbo_overlook, unsigned int* pibo_overlook);

f_int32_t qtmapNodeCreateSvsVboEs(sQTMAPNODE * pNode);
f_int32_t genSvsConstVBOs(unsigned int* pibo);

f_int32_t genVecShadeConstVBOs(unsigned int* pibo, unsigned int* ptvbo);

#ifdef _JM7200_
/*����SVS������ģʽ�¸����ӽ��µ���ʾ�б�*/
f_int32_t qtmapNodeCreateSVSGridList(sQTMAPNODE * pNode);
/*����SVS���θ澯ģʽ�¸����ӽ��µ���ʾ�б�*/
f_int32_t qtmapNodeCreateSVSList(sQTMAPNODE * pNode);
#endif


/*�����ڲ������н�ֹĿǰ�Ľڵ�����ֵ��Ϊ������*/
f_int32_t qtmapGetNodePeakValue(VOIDPtr map_handle);
/* ���ص�ǰ�������� */
f_int32_t qtmapGetBufferCount(VOIDPtr map_handle);

f_int32_t qtmapNodeGenPartGrid(sQTMAPNODE *pNode);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
