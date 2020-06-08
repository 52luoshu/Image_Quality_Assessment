//psnr.cpp -- PSNRÖ¸±ê

#include "qualitydetect.h"

double getPSNR(Mat& I1, Mat& I2)
{
    Mat s1;
    absdiff(I1, I2, s1);
    s1.convertTo(s1, CV_32FC1);
    s1 = s1.mul(s1);

    Scalar s = sum(s1);

    double sse = s.val[0]; 

    if (sse <= 1e-10) 
        return 0;
    else
    {
        double  mse = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255.0 * 255.0) / mse);
        return psnr;
    }
}
