//gssim.cpp -- GSSIM指标
//评价结果离散性与线性表现要优于SSIM

#include "qualitydetect.h"

double getGSSIM(Mat& inputimage1, Mat& inputimage2)
{
	Mat img1, img2;
	img1 = inputimage1;
	img2 = inputimage2;
	if (inputimage1.cols > 1280 || inputimage1.rows > 720)
	{
		Rect rect((int)(inputimage1.cols / 5), (int)(inputimage1.rows / 5), (int)(inputimage1.cols * 3 / 5), (int)(inputimage1.rows * 3 / 5));
		img1 = inputimage1(rect);
		img2 = inputimage2(rect);
	}

	const double C1 = 6.5025, C2 = 58.5225;
	Mat image1, image2;
	Mat gradientimg1, gradientimg2;

	img1.convertTo(image1, CV_8UC1);
	img2.convertTo(image2, CV_8UC1);

	Canny(image1, gradientimg1, 51, 204, 3);
	Canny(image2, gradientimg2, 51, 204, 3);

	gradientimg1.convertTo(gradientimg1, CV_32FC1);
	gradientimg2.convertTo(gradientimg2, CV_32FC1);
	img1.convertTo(image1, CV_32FC1);
	img2.convertTo(image2, CV_32FC1);

	Mat mu1, mu2;
	GaussianBlur(image1, mu1, Size(11, 11), 1.5);
	GaussianBlur(image2, mu2, Size(11, 11), 1.5);
	Mat mu1_2 = mu1.mul(mu1);
	Mat mu2_2 = mu2.mul(mu2);
	Mat mu1_mu2 = mu1.mul(mu2);

	Mat gradientimg1_2 = gradientimg1.mul(gradientimg1);
	Mat gradientimg2_2 = gradientimg2.mul(gradientimg2);
	Mat gradientimg1_gradientimg2 = gradientimg1.mul(gradientimg2);

	Mat mu_gimg1, mu_gimg2;
	GaussianBlur(gradientimg1, mu_gimg1, Size(11, 11), 1.5);
	GaussianBlur(gradientimg2, mu_gimg2, Size(11, 11), 1.5);
	Mat mu_gimg1_2 = mu_gimg1.mul(mu_gimg1);
	Mat mu_gimg2_2 = mu_gimg2.mul(mu_gimg2);
	Mat mu_gimg1_gimg2 = mu_gimg1.mul(mu_gimg2);

	Mat sigma_gimg1_2, sigma_gimg2_2, sigma_gimg1_gimg2;
	GaussianBlur(gradientimg1_2, sigma_gimg1_2, Size(11, 11), 1.5);
	sigma_gimg1_2 -= mu_gimg1_2;
	GaussianBlur(gradientimg2_2, sigma_gimg2_2, Size(11, 11), 1.5);
	sigma_gimg2_2 -= mu_gimg2_2;
	GaussianBlur(gradientimg1_gradientimg2, sigma_gimg1_gimg2, Size(11, 11), 1.5);
	sigma_gimg1_gimg2 -= mu_gimg1_gimg2;

	Mat t1, t2, t3;
	t1 = 2 * mu1_mu2 + C1;
	t2 = 2 * sigma_gimg1_gimg2 + C2;
	t3 = t1.mul(t2);
	t1 = mu1_2 + mu2_2 + C1;
	t2 = sigma_gimg1_2 + sigma_gimg2_2 + C2;
	t1 = t1.mul(t2);

	Mat gssim_map;
	divide(t3, t1, gssim_map);
	Scalar gssim = mean(gssim_map);
	return gssim.val[0];
}