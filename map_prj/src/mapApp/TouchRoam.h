/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2020��
**
** �ļ���: TouchRoam.h
**
** ����: ���ļ�����ͨ�������������е��ƶ�����ת�����š������Ⱥ�����ͷ�ļ���
**
** ����ĺ���:  
**
**                            
** ���ע��: 
**
** ���ߣ�
**		LPF��
** 
**
** ������ʷ:
**		2020��4��8��11:03:52 LPF �������ļ�
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#ifndef __TOUCHROAM_H__
#define __TOUCHROAM_H__


#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
/*.BH--------------------------------------------------------
**
** ������: BOOL touchRoamXY()
**
** ����:  �ƶ�����
**
** �������:  
**	state ��0 �������ƶ���1�������ƶ�
**  x, y�����ε���Ļ���꣬��Ļ���Ͻ�Ϊԭ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�1
**
**.EH--------------------------------------------------------
*/
void touchRoamXY(int state, float x, float y );

/*.BH--------------------------------------------------------
**
** ������: BOOL touchScale()
**
** ����:  ���Ų���
**
** �������:  
**	state ��0�����������ţ�1����������
**  scale��0����С��1���Ŵ�
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�2
**
**.EH--------------------------------------------------------
*/
void touchScale(int state, float scale);

/*.BH--------------------------------------------------------
**
** ������: BOOL touchYaw()
**
** ����:  ƫ������ת����
**
** �������:  
**	state ��0��������ƫ������ת��1������ƫ������ת
**  yaw��0-360��˳ʱ��Ϊ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�3
**
**.EH--------------------------------------------------------
*/
void touchYaw(int state, float yaw);

/*.BH--------------------------------------------------------
**
** ������: BOOL touchPitch()
**
** ����:  ��������ת����
**
** �������:  
**	state ��0����������������ת��1��������������ת
**  pitch��(0,-90)������Ϊ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�4
**
**.EH--------------------------------------------------------
*/
void touchPitch(int state, float pitch);

/*.BH--------------------------------------------------------
**
** ������: BOOL touchRotPointXY()
**
** ����:  ����yaw��pitch����ת���ĵ���Ļ����
**
** �������: 
**		height:	��Ļ�ĳ���	 
**		x: Ŀǰ�������ã�Ĭ������Ļ������ 
**		y����ת���ĵ���Ļ���꣬��Ļ���Ͻ�Ϊԭ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�5
**
**.EH--------------------------------------------------------
*/
void touchRotPointXY(float width ,float height, float x, float y);


/*.BH--------------------------------------------------------
**
** ������: BOOL touchMiddleDeltaSet()
**
** ����:  �������ŵĳ߶�,��λ��
**
** �������: 
**		value: ���ŵĳ߶ȣ���λ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�6
**
**.EH--------------------------------------------------------
*/
void touchMiddleDeltaSet(float value);


#endif
#endif