#include "..\include\imgproc\imgproc.hpp"
#include "..\include\core\core.hpp"
#include "..\include\highgui\highgui.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <time.h>

using namespace std;
using namespace cv;

double getPSNR(Mat& I1, Mat& I2);
double getMSSIM(Mat& img1, Mat& img2);
double getNRSS(Mat& image);
int getTime(char* out);
int getLogTime(char* out, int fmt);
int writeLog(FILE* fp, const char* str, bool bLog);
int closeLog(FILE* fp);	
int casedetect(Mat& img1, Mat& img2);
double getEntropy(Mat& image);
int iqJudge(double result, double threshold, double threshold_down);
double CSF(Mat& img1, Mat& img2);
Mat filter(Mat& img, double up, double down);
double getGSSIM(Mat& img1, Mat& img2);

