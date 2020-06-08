//casedetect.cpp -- �������
//�ж�ǰ��֡֡���ֵ�������Ϣ�أ������ж��Ǽ��������������Ȼ����

#include "qualitydetect.h"

int casedetect(Mat& img1, Mat& img2)
{
	Mat diff, diff_binary;
	absdiff(img2, img1, diff);
	threshold(diff, diff_binary, 1, 255.0, CV_THRESH_BINARY);
	double entropy = getEntropy(diff_binary);

	return (entropy >= 0.2 ? 1 : 0);	//1--��Ȼ������0--���������
}