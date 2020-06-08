//nrss.cpp -- NRSS指标
//该指标属于无参考类型，可用于监测源视频质量

#include "qualitydetect.h"

double getNRSS(Mat& image)
{
	Mat gray_img, Ir, G, Gr;
	gray_img = image;

	GaussianBlur(gray_img, Ir, Size(7, 7), 6, 6);

	//提取图像和参考图像的梯度信息
	Sobel(gray_img, G, CV_32FC1, 1, 1);//计算原始图像sobel梯度
	Sobel(Ir, Gr, CV_32FC1, 1, 1);//计算构造函数的sobel梯度

	//找出梯度图像 G 中梯度信息最丰富的 N 个图像块，n=64(即划分为8x8的大小)
	//计算每个小方块的宽/高
	int block_cols = G.cols * 2 / 9;
	int block_rows = G.rows * 2 / 9;
	//获取方差最大的block
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

	//计算结构清晰度NRSS
	double result = 1 - getMSSIM(best_G, best_Gr);

	return result;
}