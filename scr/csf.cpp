//csf.cpp -- 子波分解+CSF加权+空间相关性

#include "qualitydetect.h"

double CSF(Mat& image1, Mat& image2)
{
	Mat img1 = image1, img2 = image2;
	Mat exlowf1, exlowf2, lowf1, lowf2, midf1, midf2, heightf1, heightf2;
	exlowf1 = filter(img1, 12.75, 0); 
	exlowf1 = filter(img2, 12.75, 0);
	lowf1 = filter(img1, 38.25, 12.75); lowf2 = filter(img2, 38.25, 12.75);
	midf1 = filter(img1, 63.75, 38.25); midf2 = filter(img2, 63.75, 38.25);
	heightf1 = filter(img1, 255.0, 63.75); heightf2 = filter(img2, 255.0, 63.75);
	Mat exlowf = exlowf1 + exlowf2;
	Mat lowf = lowf1 + lowf2;
	Mat midf = midf1 + midf2;
	Mat heightf = heightf1 + heightf2;

	Mat mean1, mean2, stddev1, stddev2, covar12, mean, stddev;
	Mat R_exlow, R_low, R_mid, R_height;

	//超低频
	meanStdDev(exlowf1, mean1, stddev1); meanStdDev(exlowf2, mean2, stddev2); meanStdDev(exlowf, mean, stddev);
	divide(2, (stddev - stddev1 - stddev2), covar12);
	divide(covar12, stddev1.mul(stddev2), R_exlow);
	//低频
	meanStdDev(lowf1, mean1, stddev1); meanStdDev(lowf2, mean2, stddev2); meanStdDev(lowf, mean, stddev);
	divide(2, (stddev - stddev1 - stddev2), covar12);
	divide(covar12, stddev1.mul(stddev2), R_low);
	//中频
	meanStdDev(midf1, mean1, stddev1); meanStdDev(midf2, mean2, stddev2); meanStdDev(midf, mean, stddev);
	divide(2, (stddev - stddev1 - stddev2), covar12);
	divide(covar12, stddev1.mul(stddev2), R_mid);
	//高频
	meanStdDev(heightf1, mean1, stddev1); meanStdDev(heightf2, mean2, stddev2); meanStdDev(heightf, mean, stddev);
	divide(2, (stddev - stddev1 - stddev2), covar12);
	divide(covar12, stddev1.mul(stddev2), R_height);
	//CSF加权
	Mat w_low, w_height, w_all;
	cvAddWeighted((CvArr*)&R_exlow, 2.47, (CvArr*)&R_low, 3.16, 0, (CvArr*)&w_low);
	cvAddWeighted((CvArr*)&R_mid, 2.56, (CvArr*)&R_height, 1.0, 0, (CvArr*)&w_height);
	cvAddWeighted((CvArr*)&w_low, 1.0, (CvArr*)&w_height, 1.0, 0, (CvArr*)&w_all);

	Scalar result = cv::mean(w_all);
	double relate = result.val[0];
	return relate;
}