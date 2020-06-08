//entropy.cpp -- ��Ϣ��
//���֡�����Ϣ�أ����ڶԳ������ͽ����жϣ�������⺯���е��øú���

#include "qualitydetect.h"

double getEntropy(Mat& image)
{
    Mat img;
    cvtColor(image, img, CV_YUV2GRAY_I420);
    double temp[256] = { 0.0 };

    // ����ÿ�����ص��ۻ�ֵ
    for (int m = 0; m < img.rows; m++)
    {// ��Ч�������еķ�ʽ
        const uchar* t = img.ptr<uchar>(m);
        for (int n = 0; n < img.cols; n++)
        {
            int i = t[n];
            temp[i] = temp[i] + 1;
        }
    }

    // ����ÿ�����صĸ���
    for (int i = 0; i < 256; i++)
    {
        temp[i] = temp[i] / (img.rows * img.cols);
    }

    double result = 0;
    // ����ͼ����Ϣ��
    for (int i = 0; i < 256; i++)
    {
        if (temp[i] == 0.0)
            result = result;
        else
            result = result - temp[i] * (log(temp[i]) / log(2.0));
    }

    return result;

}