//iqJudge.cpp -- ������ֵ���Ե�ǰ��������ж�
//��Ҫ��׼ȷ����ֵ��ǰ��

#include "qualitydetect.h"

int iqJudge(double result, double threshold_up, double threshold_down)
{
	if(result<threshold_down)
	{
		return 0;	//���봦��
	}
	if ((result >= threshold_down) && (result < threshold_up))
	{
		return 1;	//�����Ԥ��
	}
	if (result >= threshold_up)
	{
		return 2;	//PASS
	}
}