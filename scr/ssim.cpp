//ssim.cpp -- SSIM指标
//NRSS计算中也调用该函数

#include "qualitydetect.h"

double getMSSIM(Mat& inputimage1, Mat& inputimage2)
{
    Mat i1, i2;
    i1 = inputimage1;
    i2 = inputimage2;
    if (inputimage1.cols > 1280.0 || inputimage1.rows > 720.0)
    {
        Rect rect((int)(inputimage1.cols / 5), (int)(inputimage1.rows / 5), (int)(inputimage1.cols * 3 / 5), (int)(inputimage1.rows * 3 / 5));
        i1 = inputimage1(rect);
        i2 = inputimage2(rect);
    }
    const double C1 = 6.5025, C2 = 58.5225;

    int d = CV_32FC1;
    Mat I1, I2;
    i1.convertTo(I1, d);
    i2.convertTo(I2, d);

    Mat I2_2 = I2.mul(I2);
    Mat I1_2 = I1.mul(I1);
    Mat I1_I2 = I1.mul(I2);
    Mat mu1, mu2;
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);
    Mat mu1_2 = mu1.mul(mu1);
    Mat mu2_2 = mu2.mul(mu2);
    Mat mu1_mu2 = mu1.mul(mu2);
    Mat sigma1_2, sigma2_2, sigma12;
    GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;
    GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;
    GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;
    Mat t1, t2, t3;
    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);
    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);
    Mat ssim_map;
    divide(t3, t1, ssim_map);
    Scalar mssim = mean(ssim_map);
    double result = mssim.val[0];
    return result;
}
