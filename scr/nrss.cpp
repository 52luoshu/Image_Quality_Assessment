//nrss.cpp -- NRSSָ��
//��ָ�������޲ο����ͣ������ڼ��Դ��Ƶ����

#include "qualitydetect.h"

double getNRSS(Mat& image)
{
	Mat gray_img, Ir, G, Gr;
	gray_img = image;

	GaussianBlur(gray_img, Ir, Size(7, 7), 6, 6);

	//��ȡͼ��Ͳο�ͼ����ݶ���Ϣ
	Sobel(gray_img, G, CV_32FC1, 1, 1);//����ԭʼͼ��sobel�ݶ�
	Sobel(Ir, Gr, CV_32FC1, 1, 1);//���㹹�캯����sobel�ݶ�

	//�ҳ��ݶ�ͼ�� G ���ݶ���Ϣ��ḻ�� N ��ͼ��飬n=64(������Ϊ8x8�Ĵ�С)
	//����ÿ��С����Ŀ�/��
	int block_cols = G.cols * 2 / 9;
	int block_rows = G.rows * 2 / 9;
	//��ȡ��������block
	Mat best_G, best_Gr;
	float max_stddev = 0.0;
	int pos = 0;
	for (int i = 0; i < 64; ++i) {
		int left_x = (i % 8) * (block_cols / 2);
		int left_y = (i / 8) * (block_rows / 2);
		int right_x = left_x + block_cols;
		int right_y = left_y + block_rows;

		if (left_x < 0) left_x = 0;
		if (left_y < 0) left_y = 0;
		if (right_x >= G.cols) right_x = G.cols - 1;
		if (right_y >= G.rows) right_y = G.rows - 1;

		Rect roi(left_x, left_y, right_x - left_x, right_y - left_y);
		Mat temp = G(roi).clone();
		Scalar mean, stddev;
		meanStdDev(temp, mean, stddev);
		if (stddev.val[0] > max_stddev) {
			max_stddev = static_cast<float>(stddev.val[0]);
			pos = i;
			best_G = temp;
			best_Gr = Gr(roi).clone();
		}
	}

	//����ṹ������NRSS
	double result = 1 - getMSSIM(best_G, best_Gr);

	return result;
}