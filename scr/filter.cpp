//filter.cpp -- ����Ҷ�任���ͨ�˲�

#include "qualitydetect.h"

Mat filter(Mat& src, double up, double down) 
{
	Mat img;
	img = src;
	//����ͼ����ٸ���Ҷ�任
	int M = getOptimalDFTSize(img.rows);
	int N = getOptimalDFTSize(img.cols);
	Mat padded;
	copyMakeBorder(img, padded, 0, M - img.rows, 0, N - img.cols, BORDER_CONSTANT, Scalar::all(0));
	//��¼����Ҷ�任��ʵ�����鲿
	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
	Mat complexImg;
	merge(planes, 2, complexImg);
	//���и���Ҷ�任
	dft(complexImg, complexImg);
	//��ȡͼ��
	Mat mag = complexImg;
	//���к��б��ż�� -2�Ķ�������11111111.......10 ���һλ��0
	mag = mag(Rect(0, 0, mag.cols & -2, mag.rows & -2));
	//��ȡ���ĵ�����
	int cx = mag.cols / 2;
	int cy = mag.rows / 2;
	//����Ƶ��
	Mat tmp;
	Mat q0(mag, Rect(0, 0, cx, cy));
	Mat q1(mag, Rect(cx, 0, cx, cy));
	Mat q2(mag, Rect(0, cy, cx, cy));
	Mat q3(mag, Rect(cx, cy, cx, cy));

	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);

	//�˲���ͨ��
	for (int y = 0; y < mag.rows; y++) {
		double* data = mag.ptr<double>(y);
		for (int x = 0; x < mag.cols; x++) {
			double d = sqrt(pow((y - cy), 2) + pow((x - cx), 2));
			if ( down < d <= up) {

			}
			else {
				data[x] = 0;
			}
		}
	}
	//�ٵ���Ƶ��
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);
	//��任
	Mat invDFT, invDFTcvt;
	idft(mag, invDFT, DFT_SCALE | DFT_REAL_OUTPUT); // Applying IDFT
	invDFT.convertTo(invDFTcvt, CV_32FC1);
	return invDFTcvt;
}