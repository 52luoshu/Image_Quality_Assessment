//casedetect.cpp -- 场景检测
//判断前后帧帧差二值化后的信息熵，粗略判断是计算机场景还是自然场景

#include "qualitydetect.h"

int casedetect(Mat& img1, Mat& img2)
{
	Mat diff, diff_binary;
	absdiff(img2, img1, diff);
	threshold(diff, diff_binary, 1, 255.0, CV_THRESH_BINARY);
	double entropy = getEntropy(diff_binary);

	return (entropy >= 0.2 ? 1 : 0);	//1--自然场景；0--计算机场景
}