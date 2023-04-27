#include "camera3d.h"

// ptCEye:�ӵ����λ��(������ӵ����ĵ��λ��);ptCCenter:(�ӵ����ĵ�λ��(�ɻ��ڵ����ϵ�λ��))
static void setCameraFollow(LP_PT_3D ptCEye, LP_PT_3D ptCCenter, LP_PT_3D ptCUp, LP_PT_3D ptCTo,
					        f_float64_t pitch, f_float64_t yaw, f_float64_t roll,
					        f_float64_t distance, LPMatrix44 rmx, LPMatrix44 mx)
{
	PT_3D ptEarthCenter;
	PT_3D ptEye, ptUp, ptUp0, ptTo;

	vector3DMemSet(&ptEarthCenter, 0 ,0 ,0);
	vector3DSetValue(&ptEye, ptCEye);
	vector3DMemSet(&ptUp, 0, 1, 0);
	vector3DMemSet(ptCCenter, mx->m[12], mx->m[13], mx->m[14]);

	// ���ڷɻ���̬������ת����
	dMatrix44RPYmemSet(rmx, pitch, yaw, roll);

	dMatrix44Multi(&ptEye, &ptEye, rmx);
	dMatrix44Multi(&ptUp0, &ptUp, rmx);	
	
	dMatrix44Multi(ptCEye, &ptEye, mx);		
	dMatrix44Multi(ptCUp, &ptUp0, mx);

	vector3DSub(ptCUp, ptCUp, ptCCenter);	
	vector3DNormalize(ptCUp);
	vector3DSub(&ptTo, ptCCenter, ptCEye);
	vector3DNormalize(&ptTo);
	vector3DSetValue(ptCTo, &ptTo);
	{
		PT_3D ptTemp;
		vector3DMul(&ptTemp, &ptTo, 20000.0);
		vector3DAdd(ptCCenter, ptCEye, &ptTemp);
	}
}
void setCameraStyle(f_uint8_t style, LP_PT_3D ptEye, LP_PT_3D ptCenter, LP_PT_3D ptUp, LP_PT_3D ptTo,
					f_float64_t pitch, f_float64_t yaw, f_float64_t roll, LPMatrix44 rmx, LPMatrix44 mx)
{
	switch(style)
	{
    case eVM_DEFAULT_VIEW:
    break;
    
	case eVM_FIXED_VIEW:		//����۲��ӽ�(б�Ϸ�30��)
		{
			vector3DMemSet(ptEye, 1000.0, 1000.0, 1000.0);		// Ĭ���ӵ�
			setCameraFollow(ptEye, ptCenter, ptUp, ptTo, 0.0, yaw , 0.0, 1000.0, rmx, mx);
			break;
		}
	case eVM_COCKPIT_VIEW:	//�����ӽ�
		{
			vector3DMemSet(ptEye, 0.0, 0.0, 10.0);		// Ĭ���ӵ�
			setCameraFollow(ptEye, ptCenter, ptUp, ptTo, pitch, yaw, roll, 10.0, rmx, mx);
			break;
		}
	case eVM_FOLLOW_VIEW:		//β���ӽ�
		{
			vector3DMemSet(ptEye, 0.0, 0.0, 1000.0);		// Ĭ���ӵ�
			setCameraFollow(ptEye, ptCenter, ptUp, ptTo, -10.0, yaw, 0.0, 1000.0, rmx, mx);
			break;
		}
	case eVM_OVERLOOK_VIEW:	//�����ӽ�,�����ʼ�ն���ǰ������,������ʱyawΪʵ�ʺ���,�汱����ʱyawΪ0,����������
		{			
			//vector3DMemSet(ptEye, 0.0, 0.0, 2000.0);		// Ĭ���ӵ� //����ģʽ����render3dMap()������
			setCameraFollow(ptEye, ptCenter, ptUp, ptTo, -90.0, yaw, 0.0, 2000.0, rmx, mx);
			break;
		}	
	default:
		break;
	}
}
